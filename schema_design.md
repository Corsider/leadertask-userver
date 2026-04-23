# Проектирование модели Goals

## Выбор коллекции
Для сущности Goal создаётся отдельная коллекция `goals` в  Mongo

## Структура документа
```json
{
  "_id": ObjectId,
  "title": "string",
  "description": "string",
  "userId": 1
}
```

## Индексы
```{ userId: 1 }``` – для выборки всех целей пользователя (GetByUser)

```_id``` – автоматически


## Остальная схема

Схема остальных сущностей может быть реализована так:

### Коллекция users
```
{
  "_id": ObjectId,
  "login": "string",
  "password": "string",
  "firstName": "string",
  "lastName": "string",
  "createdAt": ISODate
}
```

### Коллекция tasks
```
{
  "_id": ObjectId,
  "goalId": "string (hex-строка ObjectId цели)",
  "title": "string",
  "status": "string (todo | in_progress | done)",
  "createdAt": ISODate
}
```

### Индексы
```
db.users.createIndex({ login: 1 }, { unique: true });
db.goals.createIndex({ userId: 1 });
db.tasks.createIndex({ goalId: 1 });
db.tasks.createIndex({ status: 1 });
```