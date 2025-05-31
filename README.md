## StopWatch, Dead Simple, Efficient, Absolutely No Distraction

Similar to

- macOS -> [MenuStopWatch](https://github.com/shenlebantongying/MenuStopWatch_macOS) by me.
- Android -> [Tranquil Stopwatch](https://github.com/tibarj/tranquilstopwatch/)

It has 2 operations:

- Click -> Pause/Unpause
- Long Press -> Restart

The recording is stored to disk. The app doesn't need to be running in the background, and there no worries if the app is accidentally closed.


<img src="https://github.com/user-attachments/assets/10c8a03d-98d4-4d0d-8727-07c6af44eb2f" height=200>
<img src="https://github.com/user-attachments/assets/77c3c171-9b76-4547-b58f-e9e65409c111" height=200>

## Support

Feature requests will be immediately rejected. Any addition to this program is distracting bloat.


## Known Flaw

Internally, the Unix epoch time is used. It causes two problems.

1. When leap seconds happen, the stopwatch will be off by 1. However, leap seconds only happened 27 times as of 2025.
2. Pausing and restarting will cause inaccuracy for a maximum of 1 second.

https://man7.org/linux/man-pages/man2/time.2.html

## LICENSE

[GPL-3.0-or-later](https://spdx.org/licenses/GPL-3.0-or-later.html)
