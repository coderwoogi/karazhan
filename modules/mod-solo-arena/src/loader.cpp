#include "ScriptMgr.h"

#ifndef _WIN32
#include <csignal>
#endif

void AddSoloArenaScripts();

void Addmod_solo_arenaScripts()
{
#ifndef _WIN32
    // POSIX(맥/리눅스): httplib 클라이언트가 끊긴 소켓에 쓸 때 SIGPIPE 로
    // 프로세스가 즉시 종료되는 것을 방지한다(윈도우엔 SIGPIPE 자체가 없음).
    // 시련 종료 시 taunt HTTP(127.0.0.1:8000)가 브리지 다운 상태면 서버가
    // 죽던 문제 수정. 시작 시 1회 설정하면 프로세스 전역(다른 httplib 사용
    // 모듈 포함)에 적용된다.
    signal(SIGPIPE, SIG_IGN);
#endif
    AddSoloArenaScripts();
}
