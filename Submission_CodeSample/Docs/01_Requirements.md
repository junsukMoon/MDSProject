# 요구사항

## MVP 요구사항

1. Dedicated server target을 빌드할 수 있어야 합니다.
2. 서버가 Objective HP를 소유해야 합니다.
3. Objective HP 변경은 서버에서만 적용되어야 합니다.
4. 클라이언트는 replicated Objective HP를 관찰해야 합니다.
5. Mass entity는 서버/standalone authority 경로에서 생성되어야 합니다.
6. Mass movement, arrival detection, objective damage는 단계별로 구현되어야 합니다.
7. 도착한 entity는 Objective damage를 한 번만 적용해야 합니다.
8. runtime debug output은 NetMode, Objective HP, Mass state를 보여줘야 합니다.
9. Actor baseline과 Mass scenario의 profiling 비교를 기록해야 합니다.
10. 검증 결과는 문서와 로그로 남겨야 합니다.

## 검증 요구사항

- C++ 변경은 build/compile 결과를 보고합니다.
- network/replication 변경은 server/client 관찰 결과가 필요합니다.
- Objective HP 변경은 server-owned임을 확인해야 합니다.
- Mass 변경은 해당 단계의 behavior만 검증합니다.
- profiling 결과는 측정 조건과 한계를 함께 기록합니다.

## 비기능 요구사항

- 변경은 작고 리뷰 가능해야 합니다.
- AI-assisted workflow는 승인 기반으로 통제합니다.
- 문서는 면접에서 설명 가능한 수준으로 유지합니다.
