# Leadertask
Сервис для создания целей и задач на yandex userver

## Запук
```docker-compose up -d```

Сервер запустится на localhost:8080

## Тестирование
1. Запустить сервер
2. Выполнить ```python main_tests.py```
3. Получить ```ALL TESTS PASSED ✅```

## Openapi
Доступные ручки представлены в ```openapi.yaml```

## Для ДЗ 4:
Проект использует только reference подоход в Mongo, т.к. embedded не имеют смысла в контексте моего варианта задания - нет данных со схемой один-к-немногим.

Валидация в validation.js