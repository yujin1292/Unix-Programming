# Unix-Programming

## mycp , mycat, myls
Unix 명령어인 cp, cat, ls를 여러 옵션을 포함하여 기능하도록 하드코딩한 것이다.
임시파일포인터를 이용하여 구현하였다.

자세한 내용은 word 파일참고

## baseball_Game

### 설명
서버와 클라이언트사이의 통신을 통해 야구게임을 진행한다. 
Player는 최소 3명으로 3명이 입장하면 게임이 시작되고, 인원이 줄어들면 게임은 중단된다.
Player 끼리 대화가 가능하다.


![image](https://user-images.githubusercontent.com/27190708/87300662-332b9480-c549-11ea-91dd-aec3f9c69567.png)

### 실행 원리


![image](https://user-images.githubusercontent.com/27190708/87300519-f19ae980-c548-11ea-8a66-8ad9ca0f0324.png)
주석참고



### 실행사진
![baseball image](https://user-images.githubusercontent.com/27190708/87300130-4ab64d80-c548-11ea-8b20-c49c1898a241.png)




### 실행방법
터미널 이용하여 아래의 명령어를 이용하여 게임을 시작한다.

```
./server portnum 

```

```
./client ip portnum playerName
```




# Contributer 
이유진 김수빈 유연주





