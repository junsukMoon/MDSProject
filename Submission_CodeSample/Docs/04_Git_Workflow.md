# Git Workflow

## 기본 원칙

- 작은 변경 단위로 작업합니다.
- 관련 없는 refactor를 섞지 않습니다.
- 검증 결과가 있는 변경을 커밋합니다.
- 작업 범위와 커밋 메시지가 일치해야 합니다.

## 권장 흐름

1. 현재 상태 확인
2. 관련 파일 읽기
3. 계획 작성
4. 승인 후 구현
5. 검증
6. diff review
7. commit

## 커밋 메시지 예시

```text
docs: update project verification notes
feat: add objective replication debug output
chore: align coding standards and smoke verification
```

## 주의사항

- 사용자 변경을 되돌리지 않습니다.
- 명시 요청 없이 `git reset --hard`를 사용하지 않습니다.
- untracked 제출 파일이나 private memo를 커밋할지 확인합니다.
- `.private/`는 개인 메모이므로 커밋하지 않습니다.
