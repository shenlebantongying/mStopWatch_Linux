# Dead Simple, Efficient, Absolutely No Distraction StopWatch for Linux

A particular type of stopwatch.

Similar to

- macOS -> [MenuStopWatch](https://github.com/shenlebantongying/MenuStopWatch_macOS) by me.
- Android -> [Tranquil Stopwatch](https://github.com/tibarj/tranquilstopwatch/)

# Spotlight

The time is stored to disk, and the app doesn't need to be running in the background.

# Known Flaw

Internally, the Unix epoch time is used. When leap seconds happen, the stopwatch will be off by 1. However, leap seconds only happened 27 times as of 2025.

https://man7.org/linux/man-pages/man2/time.2.html
