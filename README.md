# app6-EEE4775
What to do for App 6:

# Building a Real-Time System at Your Dream Company
Scenario Overview
Imagine you’ve landed your dream job as a software engineer at a tech company in Central Florida. It could be a cutting-edge startup or an established firm – perhaps one of the innovative companies in the Orlando region. The company has tasked you with developing a real-time system for one of its key projects. This means you’ll be dealing with software where timing is as critical as correctness: the system’s functions must not only produce correct outputs, but must do so within strict time deadlines. Failure to meet those timing requirements could degrade the product’s performance or even cause a system failure, depending on how critical the task is.

# Company Context and Examples
Your choice of company and industry will shape the kind of real-time system you work on. In Central Florida’s tech hub, there are many exciting possibilities. For example, you might envision working at:

1. Aerospace/Defense (Orlando): Perhaps Lockheed Martin or a simulation startup, developing a flight control or pilot training simulator. These systems demand high responsiveness – a fighter jet’s control software or an augmented reality (AR) training system for pilots must react instantly to inputs.

2. Autonomous Vehicles (Orlando/Melbourne): A company like Luminar Technologies (an Orlando-born startup) builds LiDAR sensors and perception software for self-driving carsLinks to an external site.. Here you’d handle real-time sensor processing – the car must “see” pedestrians, other cars, and obstacles in real time to drive safely. Missing a deadline in processing could mean a collision, so many of these tasks are safety-critical.

3. Theme Park Systems (Orlando): Imagine working for Disney or Universal on roller coaster ride control systems. These involve real-time control of motors, brakes, and safety sensors. For instance, a roller coaster’s control system must synchronize sensors and actuators with precise timing to ensure a smooth and safe ride. An underlying real-time operating system (RTOS) helps guarantee instantaneous reactions to emergency conditions (e.g. an e-stop button or sensor trigger) so that the ride can be halted immediately if neededLinks to an external site.. This is absolutely vital for rider safety.

4. Healthcare Tech (Florida): You might be at a medical device company developing a patient monitoring system or an insulin pump. Such devices often have tasks with strict timing – e.g. delivering a dose at the right moment or sounding an alarm within seconds of detecting an anomaly. The software in a heart pacemaker is a classic example of a hard real-time system – missing its timing deadlines could be life-threatening.

5. Gaming/AR/VR (Orlando): Perhaps you join a local AR/VR startup or a game studio. Orlando’s tech scene includes companies like Red 6, which integrates AR into flight training, and game developers. In these domains, the system (be it a video game or an AR headset) also operates in real time: the graphics must render each frame quickly to keep up with user actions. A delay might only cause a drop in frame rate or a brief glitch rather than total failure, so these tend to be soft real-time tasks.

# Your Engineering Scenario
You’ve just been hired by a Central‑Florida tech firm — again your choice. The team’s first directive - show us what you know about real time systems by developing a proof of concept RTS relevant to the company that includes a few elements shown below.  “Build a working proof‑of‑concept in Wokwi that proves we can meet our real‑time deadlines.”

You’ll design the embedded firmware, wire up the virtual hardware, and demonstrate predictable timing.
Think of this as an internal engineering milestone: a prototype the CTO will green‑light only if it works and you can explain why it’s safe on deadline‑critical paths.

# Prototype Requirements (all implemented in Wokwi)
Area	What you must include	Notes
Hardware Sketch	ESP32, ≥ 2 external LEDs, ≥ 1 sensor, ≥ 1 momentary input	Wire & label everything in Wokwi’s diagram.json.
Tasks & ISRs	≥ 4 FreeRTOS tasks plus ≥ 1 ISR ; at least 1 task must take variable amount of time	Map each to a company use‑case (e.g., “Brake‑sensor ISR”, “LiDAR frame task”).
Timing	Declare a period/deadline for every task; mark Hard vs Soft in code comments	Hard miss = failure for your product scenario.
Synchronization	Use ≥ 2 distinct mechanisms (mutex, queue, binary sem, critical section…)	Show why each is needed.
Inter‑task & External Comms	One internal channel (queue, event group, etc.) and one outward link (UART, Wi‑Fi, I²C, …)	External link can talk to Wokwi serial monitor or a simple web page.
Determinism Proof	Log timestamps or blink a “heartbeat” LED; show all hard deadlines are met	Use Wokwi logic‑analyzeLinks to an external site.r [full credit] or serial time‑stamps [partial credit]
Company Context	In variables / comments / README: tie every task to the chosen company’s product	Classify tasks as Hard/Soft
Engineering Analysis 
(≈ 4‑6 sentences each in README)

- Scheduler Fit: How do your task priorities / RTOS settings guarantee every H task’s deadline in Wokwi? Cite one timestamp pair that proves it.

- Race‑Proofing: Where could a race occur? Show the exact line(s) you protected and which primitive solved it.

- Worst‑Case Spike: Describe the heaviest load you threw at the prototype (e.g., sensor spam, comm burst). What margin (of time) remained before an H deadline would slip?

- Design Trade‑off: Name one feature you didn’t add (or simplified) to keep timing predictable. Why was that the right call for your chosen company?

# AI Tool Policy (use if you wish)
Your company has it's own internal GenAI system - because they don't want their sensitive data ingested and possibly reused - also there's liability, if you use genAI code, you need to document it - the lawyers need to be prepared if ever called to court - so be careful how you use AI to assist you and document it when you do.

Allowed for brainstorming or code snippets.

Must list each chat/url in README AND inline with your code.

You own all final timing proofs—don’t paste code you can’t explain.

Deliver a prototype the CTO can run, inspect, and trust. Good luck building real‑time greatness!

# Submit
Item	Upload / Provide
Wokwi URL	Public link to your full simulation.
Project ZIP	Save → Download ZIP and attach for grading backup.
Source Code	Clean, commented; compiles & runs inside Wokwi.
Concurrency Diagram (1 page)	Boxes = tasks/ISR, arrows = queues/semaphores. Label periods (tasks/accesses) & Hard/Soft task.
README.md	• 75‑word company synopsis & why RT• Task table (period, H/S, consequence)• Short answers to analysis prompts (below)• AI‑usage disclosure + links if used
90‑sec Demo Video	Screen‑capture of Wokwi proving deadlines met (voiceover required).

# Rubric
Category	Pts
Working Wokwi Prototype	40
Real‑Time Design Quality	20
Concurrency Diagram & Code Clarity	10
Analysis Insight	15
Professional Context & Originality; honest AI disclosure	10
Demo Video Quality	5
