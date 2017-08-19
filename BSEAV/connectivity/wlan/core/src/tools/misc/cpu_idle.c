/* 
 * Lightweight and portable cpu idleness tool.
 *
 * Goal is to measure cpu idleness while discounting
 * the overhead of epi_ttcp or chariot endpoints (which can be quite
 * large on small processor machines).
 * Chariot spawns multiple instances of the endpoint program, so
 * handle that.
 * The user may want to keep a single version of this program running
 * while running multiple (sequential) runs of epi_ttcp, so handle the
 * case of those processes coming and going.
 * This attempts to be lightwieght and scanning for new processes is expensive
 * so only every do it every 8 iterations.  The -d option disables checking
 * for new processes entirely.
 *
 * The primary target for this is Sandgate II (An ARM based embedded machine
 * by Sophia Systems) running a 2.6.9 kernel. It does run on other systems
 * but testing is not as thorough.
 *
 * Copyright (C) 2007 Broadcom Corporation
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <libgen.h>
#include <sys/types.h>
#include <dirent.h>

char id[] = "$Revision: 12.20 $";

/* System related info */
struct info {
	char buf[256];
	char uptime_buf[64];
	char junk[16];
	unsigned long normal, nice,  system, idle, io;
	unsigned long total;
	unsigned long total_HZ;
};

/* Process related info */
struct proc_info {
	unsigned long utime;
	unsigned long stime;
	char buf[256];
};

int u_space, s_space;

#define NPROCS 15	/* Chariot spawns several processes of the same name */
			/* 'endpoint'.  This is the max number we can track */
uint pidlist[NPROCS];
uint scratchpidlist[NPROCS];

struct info stuff[2]; 			/* System counters: Current & previous */
struct proc_info proc_stuff[NPROCS][2]; /* Process counters: Current & previous */

/* Moving average calculations */
#define MA_WINDOW_SZ 4
int idle_window[MA_WINDOW_SZ];
int idle_ma,  idle_index;

#define MODINC_POW2(x, bound) (((x) + 1) & ((bound) - 1))

static inline char *
skip_ws(const char *p)
{
	while (isspace(*p)) p++;
	return (char *)p;
}

static inline char *
skip_token(const char *p)
{
	while (isspace(*p)) p++;
	while (*p && !isspace(*p)) p++;
	return (char *)p;
}

int
find_pid(char *name, int *localpidlist, int num_ents)
{
	DIR *proc_dir;
	struct dirent *proc_file;
	FILE *fp;
	char work_buf[256];
	char bname_buf[64];
	char *bname;
	int nprocs = 0;

	bzero(localpidlist, num_ents * sizeof(int));

	if ((proc_dir = opendir("/proc")) == NULL) {
		printf("Could not open /proc\n");
		exit(0);
	}

	while ((proc_file = readdir(proc_dir)) != NULL) {
		if (!isdigit(proc_file->d_name[0]))
			continue;

		sprintf(work_buf, "/proc/%s/cmdline", proc_file->d_name);
		if ((fp = fopen(work_buf, "r")) == NULL) {
			printf("Can't open /proc/%s/cmdline\n", proc_file->d_name);
			return (-1);
		}
		if (fgets(work_buf, sizeof(work_buf) -1, fp) != NULL) {

			/* Isolate just proc name, no cmd line args */
			strcpy(bname_buf, basename(work_buf));
			bname = bname_buf;
			while (*bname && !isspace(*bname)) bname++;
			*bname = 0;

			if (!strcmp(bname_buf, name)) {
				localpidlist[nprocs++] = atoi(proc_file->d_name);
			}
		}
		fclose(fp);
	}
	closedir(proc_dir);

	return (nprocs);
}


int
get_pid(char *proc_name, int *num_pids)
{
	int i, different;

	/* epi_ttcp */
	if ((*num_pids = find_pid("epi_ttcp", scratchpidlist, NPROCS)) > 0) {
		strcpy(proc_name, "epi_ttcp");
	} else {
		/* chariot */
		if ((*num_pids = find_pid("endpoint", scratchpidlist, NPROCS)) > 0) {
			strcpy(proc_name, "endpoint");
		} else {
			/* iperf */
			if ((*num_pids = find_pid("iperf", scratchpidlist, NPROCS)) > 0) {
				strcpy(proc_name, "iperf");
			}
		}
	}

	different = 0;
	for (i = 0; i < NPROCS; i++) {
		if (scratchpidlist[i] != pidlist[i])
			different = 1;
	}

	/* Copy new pidlist */
	if (different) {
		if (!num_pids)
			proc_name[0] = 0;
		for (i = 0; i < NPROCS; i++)
			pidlist[i] = scratchpidlist[i];
	}

	return (different);
}

void
header(int num_pids, char *proc_name)
{
	printf("\n");
	if (!num_pids) {
		printf("System\t\tAverage\n");
		printf("Load\t\tLoad/Idle\n");
		printf("------\t\t---------\n");
	} else {
		printf("System\tLoad from\tDiscounted\tAverage Discounted\n");
		printf("Load\t%s(%d)\tLoad\t\tLoad / Idle\n", proc_name, num_pids);
		printf("(SL)\t u    s \t");
		if (u_space && !s_space)
			printf("SL-u\n");
		if (u_space && s_space)
			printf("SL-(u+s)\n");
		printf("------\t---------\t-----\t\t----------------\n");
	}
}

void
help(char *name)
{
	printf("%s: Monitor cpu load/idleness\n", name);
	printf("[-d]\t disable checking for new processes\n");
	printf("[-u]\t subtract only user space load from System Load\n");
	printf("[-s]\t subtract system & user space load from System Load\n");
	printf("[-i interval (in seconds) for display]\n");
	printf("[-n number of iterations]\n");
}

int
main(int argc, char *argv[])
{
	char proc_fname[64];
	char proc_name[64];
	int no_check = 0;
	int interval = 2;
	int loop_cnt, nloops = -1;
	FILE *fp;
	int prev_index, cur_index;
	long net_idle, net_proc_utime, net_proc_stime, uptime_delta;
	unsigned long proc_load;
	ldiv_t system_idle_percent, proc_stime_percent, proc_utime_percent;
	char *p;
	int avg, i, num_pids, need_header;
	int ncpus = 0;
	double uptime;

	u_space = 1;
	s_space = 1;

	while (argc > 1) {
		if (!strcmp("-d", argv[1])) {
			no_check = 1;	/* Don't check for new processes */
			argv++;
			argc--;
		} else
		if (!strcmp("-u", argv[1])) {
			u_space = 1;
			s_space = 0;
			argv++;
			argc--;
		} else
		if (!strcmp("-s", argv[1])) {
			u_space = 1;
			s_space = 1;
			argv++;
			argc--;
		} else
		if (!strcmp("-v", argv[1])) {
			printf("%s\n", id);
			return (0);
		} else
		if ((argc > 2) && (!strcmp("-i", argv[1]))) {
			interval = atoi(argv[2]);
			printf("interval = %d seconds\n", interval);
			argv++;
			argv++;
			argc -= 2;
		} else
			if ((argc > 2) && (!strcmp("-n", argv[1]))) {
				nloops = atoi(argv[2]);
				printf("nloops = %d \n", nloops);
				argv++;
				argv++;
				argc -= 2;
			}
		else {
			help(argv[0]);
			return (-1);
		}
	}

	/* Read number of cpus */
	if ((fp = fopen("/proc/stat", "r")) == NULL) {
		printf("Can't open /proc/stat\n");
		return (-1);
	}
	while (fgets(proc_stuff[0][0].buf, sizeof(proc_stuff[0][0].buf) -1, fp) != NULL) {
		if (strncasecmp(proc_stuff[0][0].buf, "CPU", strlen("CPU")) == 0 &&
			isdigit(proc_stuff[0][0].buf[3]))
				ncpus++;
	}
	fclose(fp);
	proc_stuff[0][0].buf[0] = 0;

	/* Before entering main loop, search for relevent processes */
	get_pid(proc_name, &num_pids);

	header(num_pids, proc_name);
	need_header = 0;

	cur_index = 0;
	bzero(&stuff[0], sizeof(struct info));
	bzero(&stuff[1], sizeof(struct info));
	loop_cnt = 0;

	/* main loop */
	while (nloops > 0 ? loop_cnt < nloops : 1) {

		/* Swap indexes */
		prev_index = cur_index;
		cur_index = !cur_index;

		/* Read all pertinent info at once and then parse it all later
		   in order to reduce latency between reading the system info and
		   then the process info
		*/

		/* Read uptime info */
		if ((fp = fopen("/proc/uptime", "r")) == NULL) {
			printf("Can't open /proc/stat\n");
			return (-1);
		}
		if (fgets(stuff[cur_index].uptime_buf,
			sizeof(stuff[cur_index].uptime_buf) -1, fp) == NULL) {
			printf("fgets failed\n");
			return (-1);
		}
		fclose(fp);


		/* Read system info */
		if ((fp = fopen("/proc/stat", "r")) == NULL) {
			printf("Can't open /proc/stat\n");
			return (-1);
		}
		if (fgets(stuff[cur_index].buf, sizeof(stuff[cur_index].buf) -1, fp) == NULL) {
			printf("fgets failed\n");
			return (-1);
		}
		fclose(fp);

		/* Read process info */
		for (i = 0; i < NPROCS; i++) {
			if (pidlist[i]) {
				sprintf(proc_fname, "/proc/%d/stat", pidlist[i]);
				if ((fp = fopen(proc_fname, "r")) != NULL) {
					if (fgets(proc_stuff[i][cur_index].buf,
					    sizeof(proc_stuff[i][cur_index].buf) -1, fp) == NULL) {
							pidlist[i] = 0;
					}
					fclose(fp);
				} else {
					pidlist[i] = 0;
					need_header++;
				}
			}
		}

		/* Parse system info */
		p = stuff[cur_index].buf;
		p = skip_token(p);	/* Skip "cpu" */
		stuff[cur_index].normal = strtoul(p, &p, 0);
		stuff[cur_index].nice = strtoul(p, &p, 0);
		stuff[cur_index].system = strtoul(p, &p, 0);
		stuff[cur_index].idle = strtoul(p, &p, 0);
		stuff[cur_index].io = strtoul(p, &p, 0);

		/* Total all cycles */
		stuff[cur_index].total =
			stuff[cur_index].normal + stuff[cur_index].nice +
			stuff[cur_index].system + stuff[cur_index].idle +
			stuff[cur_index].io;

		/* Idle cycles delta */
		net_idle = (long)((stuff[cur_index].idle
			- stuff[prev_index].idle) & 0xffffffff);

		/* Convert to a percent */
		net_idle = net_idle * 100 / ncpus;


		/* Parse uptime */
		p = stuff[cur_index].uptime_buf;
		uptime = strtod(stuff[cur_index].uptime_buf, &p);
		stuff[cur_index].total_HZ = uptime * 100;  /* Secs to ticks */
		uptime_delta = stuff[cur_index].total_HZ - stuff[prev_index].total_HZ;

		/* Parse process info if applicable */
		num_pids = 0;
		net_proc_utime = 0;
		net_proc_stime = 0;
		for (i = 0; i < NPROCS; i++) {
			if (pidlist[i]) {
				p = proc_stuff[i][cur_index].buf;
				while (*p != ')')
					p++;
				p++;
				p = skip_token(p);  /* 1 sstate, */
				p = skip_token(p);  /* 2 sppid, */
				p = skip_token(p);  /* 3 spgid, */
				p = skip_token(p);  /* 4 ssid, */
				p = skip_token(p);  /* 5 stty_nr, */
				p = skip_token(p);  /* 6 stty_pgrp, */
				p = skip_token(p);  /* 7 stask->flags, */
				p = skip_token(p);  /* 8 stask->min_flt, */
				p = skip_token(p);  /* 9 scmin_flt, */
				p = skip_token(p);  /* 10 stask->maj_flt, */
				p = skip_token(p);  /* 11 scmaj_flt, */
				//p = skip_token(p);  /* 12 sjiffies_to_clock_t(task->utime), */
				//p = skip_token(p);  /* 13 sjiffies_to_clock_t(task->stime), */
				//p = skip_token(p);  /* 14 sjiffies_to_clock_t(cutime), */
				//p = skip_token(p);  /* 15 sjiffies_to_clock_t(cstime), */

				proc_stuff[i][cur_index].utime = strtoul(p, &p, 10);
				proc_stuff[i][cur_index].stime = strtoul(p, &p, 10);

				net_proc_utime += (long)((proc_stuff[i][cur_index].utime
					- proc_stuff[i][prev_index].utime) & 0xffffffff);
				net_proc_stime += (long)((proc_stuff[i][cur_index].stime
					- proc_stuff[i][prev_index].stime) & 0xffffffff);
				num_pids++;
			}
		}

		/*
		printf("utime %ld ", utime);
		printf("stime %ld ", stime);
		printf("tot %ld ", proc_stuff[i][cur_index].time);
		printf("net_proc_busy %d uptime_delta %ld\n", net_proc_busy, uptime_delta);
		*/

		if (need_header) {
			header(num_pids, proc_name);
			need_header = 0;
		}

		if (uptime_delta) {
			system_idle_percent = ldiv(net_idle, uptime_delta);
			printf("%.2lu%%",  100 - system_idle_percent.quot);
		} else {
			printf("-\n");
		}

		proc_load = 0;
		if (num_pids) {
			net_proc_utime *= 100;
			net_proc_stime *= 100;
			if (uptime_delta && num_pids) {
				proc_stime_percent = ldiv(net_proc_stime, uptime_delta);
				proc_utime_percent = ldiv(net_proc_utime, uptime_delta);

				if (u_space)
					proc_load += proc_utime_percent.quot;
				if (s_space)
					proc_load += proc_stime_percent.quot;

				printf("\t%.2lu%%  %.2lu%%\t%lu%%",
					proc_utime_percent.quot, proc_stime_percent.quot,
					(100 - system_idle_percent.quot) - proc_load);
			}
		} else {
			/* No processes to factor in */
			proc_utime_percent.quot = 0;
			proc_stime_percent.quot = 0;
			proc_load = 0;
		}

		/* Long term moving average */
		idle_ma -= idle_window[idle_index];
		idle_ma += system_idle_percent.quot + proc_load;
		idle_window[idle_index] = system_idle_percent.quot + proc_load;
		idle_index = MODINC_POW2(idle_index, MA_WINDOW_SZ);

		loop_cnt++;

		/* Handle case where moving average buffer has not filled up yet */
		if (loop_cnt < MA_WINDOW_SZ)
			avg = idle_ma/loop_cnt;
		else
			avg = idle_ma/MA_WINDOW_SZ;

		printf("\t\t %.2d%% / %.2d%%\n", 100 - avg, avg);

		/* Check for new pids everyother loop */
		if (!no_check && (loop_cnt & 0x1)) {
			int dont_care;
			if (get_pid(proc_name, &dont_care))
				need_header++;
		}

		sleep(interval);
	}
	return (0);
}
