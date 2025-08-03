# Company Synopsis
This real-time system is intended for autonomous vehicles as an emergency brake system. 
It will detect the vehicle's current speed, compare to the current speed limit, and warn
the user if the speed exceeds the limit due to chance of a collision. The emergency brake
button is intended to be used for reducing vehicle speed and prevent collision.



# Engineering Analysis Questions
1. testScheduler Fit: How do your task priorities / RTOS settings guarantee every H task’s deadline in Wokwi? Cite one timestamp pair that proves it.
The tasks within this program are ordered by the importance of the task. The highest priority
task in this program is the emergency brake, which is a crucial component for the functionality
of this system and its effectiveness in terms of safety. The next priorities would be
the alerts for the speeding LED and the print statements, in addition to the counting
semaphore to indicate how long the vehicle's speed has been over the limit. One timestamp
would be the period of 2s where the heartbeat task allows the green LED to blink in this interval,
even though it is a lower priority task.

2. Race‑Proofing: Where could a race occur? Show the exact line(s) you protected and which primitive solved it.
A race could occur when the emergency brake is pressed, but there are statements for the 
current speed over the limit and a warning with the SEMCNT that would occur at the same time.
These conflicts can cause the task priorities to get mixed and make it easier to miss important
alerts being printed in the serial monitor. The race proofing for this issue occurs in lines
150-153 within the event handler task. A global variable is set to determine whether the
emergency brake has been pressed or not, and the other serial prints during the speedingAlert
will not print at the same time. This ensures that the emergency brake (which is highest priority)
is the main task being output to the serial monitor.

3. Worst‑Case Spike: Describe the heaviest load you threw at the prototype (e.g., sensor spam, comm burst). What margin (of time) remained before an H deadline would slip?
Sensor spam is one of the methods used to test worst-case scenarios for this prototype,
and this can cause delays in some of the other tasks within the program. This includes spamming
through both rapidly changing the vehicle speed on the potentiometer to exceed the threshold, and 
pressing the emergency brake button multiple times within a short time frame. The heaviest
load would be the sensor spam which would just mix the print statements for the regular
vehicle speed and the over-limit speed/warning statements.

4. Design Trade‑off: Name one feature you didn’t add (or simplified) to keep timing predictable. Why was that the right call for your chosen company?
One feature not added would be the additional object detection and surrounding vehicle detection.
This program only detects the vehicle's current speed, which is a limitation due to not
detecting distance from another vehicle or 360 vehicle and object detection. However, this was
the right call, because implementation of 360 detection would require a lot of memory and
more additional tasks to be prioritized by the CPU, and it would make the program more
complex. The program also works well with utilizing an emergency brake when the speed exceeds
the provided limit, even though having object/vehicle detection would be an additional feature
that would add further to the functionality of the system.