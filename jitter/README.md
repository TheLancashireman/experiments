# Experiments::Jitter

An experiment to measure the amount of jitter in a time-triggered system.

The program compiles for a pi zero and a pi3 using the davroska OS
(see https://github.com/TheLancashireman/davros/tree/master/davroska)
but should be adaptable for any OSEK or AUTOSAR OS (subject to memory protection).

The program implements a stripped down frame based system (as was common in the avionics of the
late 70s and 80s). The aim is to measure start times of frames, start and end times of tasks
within the frames and execution time of frames under various cache and TLB cleaning strategies.
