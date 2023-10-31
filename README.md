# System programing Prj

시스템 프로그래밍 수업 내 진행했던 프로젝트 정리

## Proj 1 - MyShell 
>fork, exec system call을 사용하여 shell 구현

해당 프로젝트는 process management 관련 system call (fork, exec, wait) 과 signal handling을 이용하여 실제 shell과 동일하게 구현한다.

프로젝트는 다음 아래와 같은 phase로 진행된다.
* Phase1 
    * ls, mkdir, rm 등 기본적인 단일 bianry 실행 명령어 처리
    * /bin에 없는 built-in command 추가 (cd,exit,history)
    * command history 관리 및 재실행


* Phase2
    * pipe를 통하여 다중 명령어 실행


* Phase3
    * 명령어 background 실행
    * background -> foreground 전환 
    * foreground -> background 전환 (control-z)
    * jobs state 관리 및 출력

## Prj 2 - Concurrency Server
>다수의 clients에 response가 가능한 concurrency server를 구현한다.

해당 concurrency 문제를 해결하기 위해 event-driven, multi-thread 두가지 방법을 적용한다.

task별로 request를 처리해주는 방식만 다르게 구현하고 내부 endpoint는 동일하게 구현한다. client 요청에 따른 stock 정보에 대한 read, write 구현은 각각의 주식을 하나의 노드로 binary tree를 통해 접근하여 수행한다.

### Task1 : Event-driven Approach
Select system call을 사용하여 accept된 socket의 변화(client write)를 실시간으로 확인하여 해당 request를 처리해준다. 

### Task2 : Thread-based Approach
multi-thread 기반으로 client accept시, 해당 연결동안 통신을 담당할 thread를 생성하여 하나의 연결당 하나의 thread를 mapping하여 처리한다.

task1에서는 하나의 flow로 순차적으로 stock read, write가 발생하기에 race condition이 발생하지 않으나, multi thread 환경에서는 여러 thread가 동시에 같은 stock에 접근시 순차적으로 처리해주기 위해 해당 thread들을 semaphore를 사용하여 lock과 unlock을 진행한다.

### Task3 : Performance Evaluation
task1과 task2에서 구현한 서버의 성능을 비교한다. 추가로 thread를 미리 생성해 연결 시 시작하는 thread pool과 연결시마다 thread를 생성하는 multi-thread 두가지를 비교한다.



* client 수에 따른 처리 속도 비교

|client cnt| 200 | 300 | 400 | 500 |
|----------|-----|-----|-----|-----|
|multi thread| 10.06087 | 10.2592 |10.3811075| 10.415742|
|select| 10.119035| 10.3338 |10.516355| 10.614366|
|thread pool| 10.05506| 10.21925| 10.304645| 10.365644|


 
![그림](https://github.com/ljy2855/system_programing/assets/10630330/e6d8b6c4-bdf1-4e28-8475-c64046828b53)