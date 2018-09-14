---
layout: default
---

# Milestone 1:

## Objectives:
- A robot that successfully follows a line
- A robot that successfully traverses a grid in a figure eight

## Line Following

We added 2 IR sensors to the robot that can sense a white line. When following a line, these two sensors hover just above the right and left edges of the line. Then, if the robot drifts left, the right sensor will be sensing the white line and vice versa if the robot drifts right. So if we find the right line sensor is seeing the white line, we speed up the left servo and slow down the right servo to compensate for the left drift until right sensor is no longer over the white line. If the left sensor is seeing the white line the same procedure is done except in the opposite direction.

![Line Following Code](https://github.com/soapbar/team8s/images/milestone1/line_following.png)

## Figure Eight
