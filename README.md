# System programing Project

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

## Proj 2 - Concurrency Server
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

## Proj3 - Malloc Lab
>heap 영역에 동적인 메모리 할당을 진행하는 malloc system call을 직접 구현해보면서 메모리 할당 및 해제 시 효율적인 utilization 과 throughput을 고려한 allocator를 구현한다.

### Implicit Allocator
먼저 바탕이 되는 기본 allocator를 구현한다. block의 header과 footer에
블럭의 사이즈를 정하여 다음 블럭과 이전 블럭의 위치를 계산할 수 있는
구조로 구현한다. 또한 header의 lsb에는 해당 블럭이 할당되었는지 여부를
저장한다.


메모리 할당 시에 heap의 head부터 비어있는 block을 확인해가며 충분히
할당할 수 있는 블럭을 linear search로 찾게 된다. 만약 충분한 block이
없다면 mem_sbrk()함수를 통해 전체 heap size을 늘리게 된다.


메모리의 할당을 해제할 때, 해당 블럭의 allocate bit을 초기화 시켜준다.
또한 양 옆에 free된 block이 있다면 해당 블럭과 합치는 과정을 진행한다.
 
 
### Explicit Allocator
implicit allocator에서 explicit allocator으로 확장한다. implicit에서 메모리를 할당할 때, head부터 시작하여 next block으로 이동하며 해당 block이 할당이 되어 있는지 할당을 위한 사이즈가 충분한지를 확인한다. 

explicit
방식은 free된 block들을 linked list로 관리하여 2 WORD의 블럭에 각각의
prev, next 포인터를 저장하여 이전과 이후의 free block의 주소를
저장해준다. 때문에 새로운 메모리를 할당할 때, free된 list중에서 linear
search를 통해 적당한 size의 block을 선택하여 할당하게 된다.


free된 블럭을 따로 관리하기 때문에 coalesce시에 추가적으로 linked list을
업데이트해주는 과정을 추가해준다. 마찬가지로 메모리 할당 시 allocate
size보다 큰 block에 할당하는 경우 남은 블럭을 새로운 free block으로
추가하고 이를 list에다 넣어준다.


### Realloc
malloc과 free가 구현이 완료 후 구현을 진행한다. realloc 구현에서 가장 큰
목표는 memory utilization을 올리는 것이다. 기존의 block에서 size를
늘리는 경우 해당 블럭을 free하고 새로운 블럭을 할당하고 기존의
payload를 복사하게 된다. 이렇게 구현하는 경우 조금의 사이즈를 차츰
늘려가는 case의 경우 매번 새로운 malloc과 free를 해야 하기에
utilization과 throughput 모두 떨어지는 결과를 갖는다.


이를 해결하기 위해 realloc 시 coalesce와 마찬가지로 양옆의 free된 블럭을
확인하고 만약 해당 prev, next 블럭의 사이즈만으로도 realloc 할당이
가능할 경우 free와 malloc없이 size를 조절한다.


추가적으로 heap의 tail에 가까운 즉 가장 마지막의 block 사이즈를 늘리게
될 경우 새로운 malloc을 하기보다 필요한 크기만큼 heap을 extend하여
해당 realloc을 할당해주게 구현한다.


### Swap split place
할당할 블럭의 사이즈가 free block size보다 최소 블럭 사이즈보다 작게
된다면 해당 free block은 필요한 만큼만 할당하고 필요없는 부분은 free
block으로 추가해주게 된다. 이때 단순히 free block의 앞부분에만 할당하게
된다면 일부 작은 크기의 블럭들이 free 되었을 때, 큰 블록들 사이에서
할당하기에 애매한 사이즈로 남게 되어 memory utilization이 떨어지게
된다.


이를 해결하기 위해 일정 크기 이상의 블럭들은 할당시 tail과 가깝게, 일정
크기 이하의 블럭은 head에 가깝게 할당한다. 이러면 작은 블럭들을 한편에
모아 free되었을 때, coalesce가 되기 유리하게 구현한다.


### Performance
![그림](https://github.com/ljy2855/system_programing/assets/10630330/85d04937-fe4c-4003-b69f-85a633ce49c6)