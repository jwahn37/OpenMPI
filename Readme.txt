120180389 안진우 분산 프로그래밍 과제 1

프로그램은 다음과 같은 순서로 실행한다.

1. 프로그램을 컴파일한다.

$make

2. 4번 문제를 위해 구현한 hw_prefixsum_1, hw_prefixsum_2, hw_prefixsum_3을 다음과 같이 수행한다.

$sh run.sh $(NUM_CORE) hw_prefixsum_1 $(NUM_PREFIX)
$sh run.sh $(NUM_CORE) hw_prefixsum_2 $(NUM_PREFIX)
$sh run.sh $(NUM_CORE) hw_prefixsum_3 $(NUM_PREFIX)

여기서 NUM_CORE는 원하는 코어의 수를 직접 입력한다.
NUM_PREFIX는 prefixsum을 구할 random 값의 갯수를 입력한다.
여기서 NUM_CORE은 2의 n제곱승이어야 하며, NUM_PREFIX는 반드시 NUM_CORE의 배수여야 한다.

3. 5번 문제를 위해 구현한 hw_imgprocess_1, hw_imgprocess_2를 다음과 같이 수행한다.

$echo $(FILE_NAME) | ./hw_imgprocess_1
$echo $(FILE_NAME) | sh run.sh ($NUM_CORE) hw_imgprocess_2

여기서 FILE_NAME은 이미지 프로세싱을 원하는 파일의 위치를 의미한다. 
여기서 NUM_CORE은 2이상이어야 한다. 
master노드와 slave노드를 최소한 각각 1개씩 할당해야 하기 때문이다.

만일, ppm_example 폴더에 저장된 모든 파일에 대해서 이미지 프로세싱을 수행하려면 다음과 같이 입력한다. 이 때 이미지프로세싱된 이미지는 같은 경로에 생성된다.

$sh hw_imgprocess_1.sh
$sh hw_imgprocess_2.sh $(NUM_CORE)
 