#! /usr/bin/env python

import random, os, signal

def sigs (signum, frame):
    print "caught ", signum

signal.signal (signal.SIGALRM, sigs)

f = random.choice (os.listdir ("/home/atw/tfdemo/memes"))

os.system ("eog /home/atw/tfdemo/memes/" + f + "& sleep 3; kill $!")
