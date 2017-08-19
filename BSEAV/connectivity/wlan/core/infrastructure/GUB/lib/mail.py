"""Handle email notifications."""

from __future__ import print_function
from __future__ import unicode_literals

import email.mime.multipart
import email.mime.text
import os
import smtplib
import sys
import traceback

import lib.consts

SMTPHOST = 'smtphost.' + lib.consts.DOMAIN


def domainify(addrs):
    """Add the local domain to any addresses which don't have it."""
    naddrs = []
    if not addrs:
        return naddrs

    seen = set()
    for addr in addrs:
        if addr.lower() == 'none':
            continue
        if addr in seen:
            continue
        seen.add(addr)
        if '@' in addr:
            naddrs.append(addr)
        else:
            naddrs.append('@'.join([addr, lib.consts.DOMAIN]))

    return naddrs


class Msg(object):

    """Handle email notifications."""

    def __init__(self, subject='testing', filtertype=None,
                 to=None, cc=None, bcc=None, reply_to=None,
                 body='', files=None, html=False, via=False):
        self.headers = {}
        self.subject = subject
        fromname = lib.consts.STATE.by
        self.to = domainify(to)
        self.cc = domainify(cc)
        self.bcc = domainify(bcc)
        if not (self.to or self.cc or self.bcc):
            self.to = domainify([fromname])

        if via:
            self.sender = '%s via GUB <%s@%s>' % (fromname, fromname,
                                                  lib.consts.DOMAIN)
        else:
            self.sender = 'GUB <%s@%s>' % (fromname, lib.consts.DOMAIN)

        if reply_to:
            self.headers['Reply-To'] = domainify(reply_to)[0]
        elif self.to:
            self.headers['Reply-To'] = self.to[0]

        self.body = body
        self.html = html

        if files:
            if self.body:
                self.body = self.body.rstrip() + '\n\n' + '-' * 32 + '\n\n'
            for fn in files:
                if len(files) > 1:
                    self.body += '\n\n' + os.path.basename(fn) + ':\n'
                try:
                    with open(fn) as f:
                        self.body += f.read()
                except Exception:
                    traceback.print_exc()

        # To aid in filtering.
        if filtertype:
            self.headers['X-GUB-' + filtertype] = fromname
        else:
            self.headers['X-GUB'] = fromname

        self.headers['Precedence'] = 'list'
        self.headers['X-Auto-Response-Suppress'] = 'OOF'

    def send_(self):
        """Deliver the message via SMTP."""
        # Allow an explicit address of 'None' to suppress all email.
        if self.to and not self.to[0].lower().startswith('none@'):
            msg = email.mime.multipart.MIMEMultipart()
            for header in sorted(self.headers):
                msg.add_header(header, self.headers[header])
            msg['Subject'] = self.subject
            msg['From'] = self.sender
            msg['To'] = ','.join(self.to)
            if self.cc:
                msg['Cc'] = ','.join(self.cc)
            # Do not add a Bcc header. A bcc happens when an address
            # is in the recipients list without being in a header.

            if self.html:
                hbody = '\n%s\n%s\n%s\n' % ('<html><body><pre>',
                                            self.body.strip(),
                                            '</pre></body></html>')
                msg.attach(email.mime.text.MIMEText(hbody, 'html'))
            else:
                msg.attach(email.mime.text.MIMEText(self.body, 'plain'))

            recipients = sorted(set(self.to + self.cc + self.bcc))
            for i, addr in enumerate(recipients):
                if '@' not in addr:
                    recipients[i] = '@'.join([addr, lib.consts.DOMAIN])

            # Failure to send email (eg mailhost down) need not
            # result in build failure.
            try:
                s = smtplib.SMTP(SMTPHOST)
                s.sendmail(self.sender, recipients, msg.as_string())
                s.quit()
            except Exception as e:
                sys.stderr.write('%s\n' % e)
                return False

        return True

    def send(self):
        """Deliver the message via SMTP."""
        # We don't want builds to fail when email is unavailable but at the
        # same time we really like to get the mail. So try a few times.
        for _ in range(5):
            if self.send_():
                return True
        return False

# vim: ts=8:sw=4:tw=80:et:
