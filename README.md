# Timer
 A timer to handle timed task.

 ```
 timer::Timer timer;
 timer.AddTask([](){printf("hello world!")}, 0);
 timer.AddTask([](){printf("hello world!")}, 10);
 timer.AddTask([](){printf("hello world!")}, 100);
 timer.AddTask([](){printf("hello world!")}, 200);
 timer.AddTask([](){printf("hello world!")}, 1000);
 ```
