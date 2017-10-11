function h_all = acphy_plot_chanest(fname, nsts, standard, bandwidth, fignum, comment)
% function htphy_plot_chanest(fname, nsts, leg_ofdm, fignum, comment)
%   - plot channels retrieved from htphy chanest memories
%   - see also: htphyprocs.tcl, proc htphy_dump_chanest
% Inputs:
%    fname - input mfile with the chanest info. 
%            If apple.m has the chanest dump, fname = 'apple'
%    nsts - number of space-time streams (default = 4)
%    standard - indicate whether this chanest dump is legacy ofdm ('dot11ag'),
%              dot11n ('bcm1') or 11ac frame ('vht') (default = 'vht')
%    bandwidth - valid inputs '20', '40', '20L', '20U' 
%                (default = '20' for BW=20 and '40' for BW=40)   
%    fignum - specify the figure number for plotting
%    comment - specify the comment on the figure


%close all
set(0, 'defaultfigurevisible', 'off');
#eval(fname);
# Matlab can't eval dots in commands, so read as a file.
fid = fopen(strcat(fname,'.m'));
source = fread(fid,[1 inf],'*char');
fclose(fid);
eval(source);

nfft = size(chan, 3);

if (~exist('nsts') || isempty(nsts))
    nsts = 4;
end
if (~exist('standard') || isempty(standard))
    standard = 'vht';
end
if (~exist('bandwidth') || isempty(bandwidth))
    if nfft == 256
        bandwidth = '80';
    elseif nfft == 128
        bandwidth = '40';
    else
        bandwidth = '20';
    end
end
if (~exist('fignum') || isempty(fignum))
    fignum = 100;
end
if (~exist('comment') || isempty(comment))
    comment = '';
end
if (~exist('figname') || isempty(figname))
    figname = "./"
end

h_all = chan;
h_all(:, nsts+1:end, :) = 0;

nrx = size(h_all,1);

% Null Out the Unused Tones
if strcmpi(bandwidth,'80')
    unused_tones = [-128:-123 -103 -75 -39 -11 -1:1 11 39 75 103 123:127] +129;
    used_tones = setdiff([1:nfft],unused_tones);
    # flip sign between -20 and 0
    for k = [nfft/4+1:nfft/2]
      h_all(:, :, k) = -h_all(:, :, k);
    end
elseif strcmpi(bandwidth,'40')
    if strcmpi(standard, 'vht')
        unused_tones = [-64:-59 -53 -25 -11 -1:1 11 25 53 59:63] +65;
        used_tones = setdiff([1:nfft],unused_tones);
    else
        used_tones = [7:63 67:123];
    end
else
    if strcmpi(standard, 'vht')
        unused_tones = [-32:-29 -21 -7 0 7 21 29:31] +33;
        used_tones = setdiff([1:nfft],unused_tones);
    elseif strcmp(standard,'dot11ag')
        %Legacy 20 or Legacy 20L
        used_tones = [7:32 34:59];
    else
        %HT 20 or Legacy 20L
        used_tones = [5:32 34:61];
    end
    
    if strcmpi(bandwidth,'20U')
        % (20U is "upshifted" by nfft/2 compared to 20L)
        used_tones = used_tones + 64; 
    end
end


%Plot magnitude thumb
figure(fignum);
for rx = 1:nrx
    for tx = 1:nrx
            subplot(nrx, nrx, (rx-1)*nrx + tx);
            %axis equal;
            %axis square;
            val = 10*log10(abs((squeeze(h_all(rx, tx, used_tones)))).^2);
            xaxis=[-nfft/2:nfft/2-1]*10/32;
            plot(xaxis(used_tones), val, '-');
            axis([-nfft/2*10/32 nfft/2*10/32 -50 15]);
            axis("nolabel", "off");
            %grid on;
    end
end

print('-dpng', "-S62,13", strcat(figname ,'magnitude_sm.png'));

%  for ctr = 1:nrx,
%    subplot(nrx, nrx, (ctr-1)*nrx + 1);
%    ylabel(sprintf('Magnitude: RX%d',ctr-1));
%  end
%  for ctr = 1:nrx,
%    subplot(nrx, nrx, nrx*(nrx-1)+ctr);
%    xlabel('f (MHz)');
%    subplot(nrx, nrx, ctr);
%    title(sprintf('STS%d', ctr-1));
%  end
				%Plot magnitude
fignum = fignum + 1;
figure(fignum);
for rx = 1:nrx
    for tx = 1:nrx
            subplot(nrx, nrx, (rx-1)*nrx + tx);
            axis equal;
            axis square;
            val = 10*log10(abs((squeeze(h_all(rx, tx, used_tones)))).^2);
            xaxis=[-nfft/2:nfft/2-1]*10/32;
            plot(xaxis(used_tones), val, '.');
            axis([-nfft/2*10/32 nfft/2*10/32 -50 15]);
            grid on;
    end
end

for ctr = 1:nrx,
    subplot(nrx, nrx, (ctr-1)*nrx + 1);
    ylabel(sprintf('Magnitude: RX%d',ctr-1));
end
for ctr = 1:nrx,
    subplot(nrx, nrx, nrx*(nrx-1)+ctr);
    xlabel('f (MHz)');
    subplot(nrx, nrx, ctr);
    title(sprintf('STS%d', ctr-1));
end
print('-dpng', strcat(figname ,'magnitude.png'));


%Plot phase
%Remove Cyclic Shift
%DC tone has 0 phase offset
%x=0;
%y=pi/4;
%z=pi/8;
%for m=1:nfft
%    for k = 1:nrx
%        h_all(:,nrx,m)=h_all(:,1,m)*exp(-j*x*(m-nfft/2-1));
%    end
%end
cdd=[0, pi/4, pi/8, pi/8*3];
for m=1:nfft
  for jj=1:nrx
    h_all(:,jj,m)=h_all(:,jj,m)*exp(-j*cdd(jj)*(m-nfft/2-1));
  end
end

%Remove time offset
x=squeeze(h_all(1,1,:));
x=angle(sum(conj(x(5:end-6)).*x(6:end-5)));
for m=1:nfft
    h_all(:,:,m)=h_all(:,:,m)*exp(-j*x*m);
end

fignum = fignum + 1;
figure(fignum);
for rx = 1:nrx
    for tx = 1:nrx
            subplot(nrx, nrx, (rx-1)*nrx + tx);
            axis equal;
            axis square;
            val = angle((squeeze(h_all(rx, tx, :))));
            xaxis=[-nfft/2:nfft/2-1]*10/32;
            plot(xaxis(used_tones), (val(used_tones)), '.',
		 'MarkerSize', 0.5); 
            axis([-nfft/2*10/32 nfft/2*10/32 -pi pi]);
	    axis("nolabel", "off");
            %grid on;
    end
end
print('-dpng', "-S62,13", strcat(figname ,'phase_sm.png'));

fignum = fignum + 1;
figure(fignum);
for rx = 1:nrx
    for tx = 1:nrx
            subplot(nrx, nrx, (rx-1)*nrx + tx);
            axis equal;
            axis square;
            val = angle((squeeze(h_all(rx, tx, :))));
            xaxis=[-nfft/2:nfft/2-1]*10/32;
            plot(xaxis(used_tones), (val(used_tones)), '.');
            axis([-nfft/2*10/32 nfft/2*10/32 -pi pi]);
	    set(gca,'YTick',-pi:pi/2:pi)
	    set(gca,'YTickLabel',{'-pi','-pi/2','0','pi/2','pi'})
	    grid on;
    end
end

fignum = fignum + 1;
for ctr = 1:nrx,
    subplot(nrx, nrx, (ctr-1)*nrx + 1);
    ylabel(sprintf('angle: RX%d',ctr-1));
end
for ctr = 1:nrx,
    subplot(nrx, nrx, nrx*(nrx-1)+ctr);
    xlabel('f (MHz)');
    subplot(nrx, nrx, ctr);
    title(sprintf('STS%d', ctr-1));
end
print('-dpng', strcat(figname ,'phase.png'));


%Plot condition Number
% FIXME: Hack - this code doesn't infer Nsts or use Nsts consistently.
fignum = fignum + 1;
figure(fignum);
cnum = [];
for nsts_ctr = 1:nrx
    h_sum = sum(sum(h_all(:,nsts_ctr,:)));
    if(h_sum == 0)
        % If current nsts ctr gives zero H, true nsts = current nsts -1
        inferred_nsts = nsts_ctr -1;
        break;
    else
        inferred_nsts = nsts_ctr;
    end
end
    
for ctr = 1:size(h_all,3),
    ht = squeeze(h_all(:,1:inferred_nsts,ctr));
    cnum(ctr) = 10*log10(cond(ht));
end
subplot(1,1,1);
plot(xaxis(used_tones), cnum(used_tones), '-');
axis("nolabel", "off");
%axis([-nfft/2*10/32 nfft/2*10/32 0 20]);
print('-dpng', "-S62,13", strcat(figname ,'condition_sm.png'));
  
fignum = fignum + 1;
figure(fignum);
plot(xaxis(used_tones), cnum(used_tones), '.-');
grid on;
xlabel('f (MHz)');
ylabel('10*log_{10}(cond(H_k))');
title(sprintf('%s: Condition Number of H_k (%d, %d) %sMHz',
	      comment, nfft, nsts, bandwidth));
print('-dpng', strcat(figname ,'condition.png'));

printf("mean/min/max=%f/%f/%f\n",
       mean(cnum(used_tones)), min(cnum(used_tones)), max(cnum(used_tones)));

endfunction
