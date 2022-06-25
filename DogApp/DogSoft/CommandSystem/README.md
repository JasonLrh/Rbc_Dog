# command excute

1. First Word  
   
|char|function|
|:-:|:-|
|E|force stop all motor|
|M|motor settings|
|S|system settings|
|L|leg settings|
|O|output settings|

---
## `M`(Motor)

1. first word: select

|char|select|
|:-:|:-|
|1~8|select single motor of number|
|A|select all|

2. second word: method

|char|method|
|:-:|:-|
|G|get motor [`position`,,,]|
|S|set `Z` for zero-position fix|
|F|function|

~~~sh
$ M1G
# get motor1's state
> [M1]  0.32    0.58    0.14

$ M3SZ0.23
# please don't use. not work
~~~

3. funciton space

|char|method|
|:-:|:-|
|M|`Z`:zeroPosition,<br>`R`:Reset_mode(no output),<br>`M`:working|
|A|{`float`}, set angle|
|P|{f f f f f},<br>set target{pos,speed},<br>k{p,v} and current|

---
## `S`(System)
1. `S`

|char|method|
|:-:|:-|
|W|enable step generator<br>(main task)|
|U|set kp & kv && standup|
|D|sitdown|

~~~sh
# only this command let mainTask run
$ SSW

# below command will stop mainTask

## sitdown
$ SSD

## standup
$ SSU0.2,0.4 # or SSU-1,-1
~~~

