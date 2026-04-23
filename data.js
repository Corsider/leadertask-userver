db = db.getSiblingDB("goals_db");

db.goals.drop();

db.goals.insertMany([
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f000" },
    "title": "Выучить userver",
    "description": "Пройти все туториалы",
    "userId": 1
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f001" },
    "title": "Написать REST API",
    "description": "CRUD для целей и задач",
    "userId": 1
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f002" },
    "title": "Спорт",
    "description": "Регулярные тренировки",
    "userId": 2
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f003" },
    "title": "Книги",
    "description": "Прочитать 20 книг за год",
    "userId": 2
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f004" },
    "title": "Пет-проект",
    "description": "Закончить Todo на Mongo",
    "userId": 1
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f005" },
    "title": "Английский",
    "description": "Уровень B2",
    "userId": 3
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f006" },
    "title": "Документация",
    "description": null,
    "userId": 3
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f007" },
    "title": "Автоматизация",
    "description": "CI/CD для сервиса",
    "userId": 1
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f008" },
    "title": "Бюджет",
    "description": "Вести учёт расходов",
    "userId": 4
  },
  {
    "_id": { "$oid": "60b8d5f9d4f0b2a1c8d5f009" },
    "title": "Отпуск",
    "description": "Спланировать маршрут",
    "userId": 4
  }
]);

