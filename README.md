# bank_chat_string-calculator_omok
서버와 클라이언트로 구현한 은행 입출금, 조회 시스템 + 오목게임 + 채팅 및 클라이언트 문자열 계산기 기능추가,
5초마다 데이터 송수신이 발생안할경우 서버에서 클라이언트에 HB전송으로 세션체크, 5회이상 응답없을 경우 세션종료,
client.cfg에 계좌번호, 비번, 잔고정보를 저장후 정보가 없데이트 될경우clientup.cfg별도 저장, 
오목게임은 테이블 최대10개 해당 테이블에 먼저입장한 사람이 선수 
 
컴파일후 테스트 해보시고 추가하고 싶은 기능이 있으면 추천해 주세요\
gcc -lpthread -o client bankcli_omok_cal.c\
gcc -lpthread -o server bankser_omok.c
