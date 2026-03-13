➜  pristine (test) wrk -t4 -c200  -d30s -H "Host: example.com" http://127.0.0.1:19080/                     ✭ ✱
wrk -t1 -c1000 -d30s -H "Host: example.com" http://127.0.0.1:19080/
Running 30s test @ http://127.0.0.1:19080/
  4 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    10.00ms    3.59ms  97.15ms   93.98%
    Req/Sec     5.03k   768.45     6.64k    72.64%
  597958 requests in 30.10s, 61.59MB read
  Socket errors: connect 0, read 597957, write 0, timeout 0
Requests/sec:  19865.89
Transfer/sec:      2.05MB
Running 30s test @ http://127.0.0.1:19080/
  1 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    51.19ms   11.85ms 213.03ms   90.55%
    Req/Sec    19.50k     3.29k   24.99k    76.85%
  578552 requests in 30.10s, 59.59MB read
  Socket errors: connect 0, read 578550, write 0, timeout 0
Requests/sec:  19221.98
Transfer/sec:      1.98MB
➜  pristine (test) wrk -t16 -c10000  -d30s -H "Host: example.com" http://127.0.0.1:19080/                  ✭ ✱
wrk -t1 -c1000 -d30s -H "Host: example.com" http://127.0.0.1:19080/
Running 30s test @ http://127.0.0.1:19080/
  16 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   273.40ms  220.55ms   2.00s    94.57%
    Req/Sec     1.23k   551.13     5.76k    84.74%
  585289 requests in 30.06s, 60.28MB read
  Socket errors: connect 0, read 585278, write 1, timeout 10279
Requests/sec:  19467.52
Transfer/sec:      2.01MB
Running 30s test @ http://127.0.0.1:19080/
  1 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    42.31ms   10.06ms 244.32ms   82.72%
    Req/Sec    20.07k     5.48k   25.97k    89.19%
  590917 requests in 30.05s, 60.86MB read
  Socket errors: connect 0, read 590916, write 0, timeout 0
Requests/sec:  19662.31
Transfer/sec:      2.03MB
➜  pristine (test) wrk -t16 -c1000  -d30s -H "Host: example.com" http://127.0.0.1:19080/                   ✭ ✱
Running 30s test @ http://127.0.0.1:19080/
  16 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    50.29ms    8.61ms 120.27ms   87.61%
    Req/Sec     1.24k   283.16    11.38k    85.86%
  592123 requests in 30.09s, 60.99MB read
  Socket errors: connect 0, read 592112, write 0, timeout 0
Requests/sec:  19680.49
Transfer/sec:      2.03MB
➜  pristine (test) wrk -t8 -c10000  -d30s -H "Host: example.com" http://127.0.0.1:19080/                   ✭ ✱
Running 30s test @ http://127.0.0.1:19080/
  8 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   253.09ms  222.05ms   2.00s    94.37%
    Req/Sec     2.60k   638.14    11.77k    78.98%
  615313 requests in 30.08s, 63.38MB read
  Socket errors: connect 0, read 615302, write 0, timeout 10357
Requests/sec:  20457.80
Transfer/sec:      2.11MB
➜  pristine (test)            




➜  pristine (test) wrk -t8 -c100  -d30s -H "Host: example.com" http://127.0.0.1:19080/                     ✭ ✱
Running 30s test @ http://127.0.0.1:19080/
  8 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     4.23ms    1.24ms  60.03ms   87.64%
    Req/Sec     2.82k   256.37     5.23k    84.94%
  674540 requests in 30.06s, 69.48MB read
  Socket errors: connect 0, read 674532, write 0, timeout 0
Requests/sec:  22442.42
Transfer/sec:      2.31MB
➜  pristine (test) wrk -t8 -c100000  -d30s -H "Host: example.com" http://127.0.0.1:19080/                  ✭ ✱
Running 30s test @ http://127.0.0.1:19080/
  8 threads and 100000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   123.25ms  202.05ms   1.50s    97.06%
    Req/Sec     3.24k     3.96k    9.60k    83.33%
  3740 requests in 0.99m, 394.45KB read
  Socket errors: connect 71763, read 1670, write 0, timeout 0
Requests/sec:     62.93
Transfer/sec:      6.64KB
➜  pristine (test) wrk -t16 -c100000  -d30s -H "Host: example.com" http://127.0.0.1:19080/                 ✭ ✱
Running 30s test @ http://127.0.0.1:19080/
  16 threads and 100000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   334.21ms  509.84ms   1.51s    81.10%
    Req/Sec     1.10k     2.54k    9.82k    93.33%
  4296 requests in 0.92m, 453.09KB read
  Socket errors: connect 71749, read 1799, write 0, timeout 0
Requests/sec:     78.03
Transfer/sec:      8.23KB
➜  pristine (test) wrk -t16 -c50000  -d30s -H "Host: example.com" http://127.0.0.1:19080/                  ✭ ✱
Running 30s test @ http://127.0.0.1:19080/
  16 threads and 50000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   977.40ms  515.36ms   1.97s    59.13%
    Req/Sec   497.33      1.16k    6.44k    93.68%
  45522 requests in 33.83s, 4.69MB read
  Socket errors: connect 21769, read 45034, write 0, timeout 17277
Requests/sec:   1345.76
Transfer/sec:    141.94KB
➜  pristine (test) wrk -t16 -c30000  -d30s -H "Host: example.com" http://127.0.0.1:19080/                  ✭ ✱
Running 30s test @ http://127.0.0.1:19080/
  16 threads and 30000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   997.24ms  453.65ms   2.00s    67.96%
    Req/Sec   397.22      1.02k    9.95k    95.43%
  91250 requests in 33.71s, 9.40MB read
  Socket errors: connect 1770, read 91133, write 0, timeout 36255
Requests/sec:   2706.78
Transfer/sec:    285.48KB