# Mongo запросы

## 1. Создание цели
```js
db.goals.insertOne({
  title: "Новая цель",
  description: "Описание",
  userId: 1
})
```

## 2. Получить цель по ID
```js
db.goals.findOne({ _id: ObjectId("60b0") })
```

## 3. Список целей пользователя
```js
db.goals.find({ userId: 1 }).sort({ _id: -1 }).toArray()
```

## 4. Список целей юзера
```js
db.goals.find({ userId: 1 }).sort({ _id: -1 }).toArray()
```

## 5. Создание юзера
```js
db.users.insertOne({
  login: "$1",
  password: "$2",
  firstName: "$3",
  lastName: "$4",
  createdAt: new Date()
})
```

## 6. Получение юзера по логину
```js
db.users.findOne({ login: "$1" })
```

## 7. Получение юзера по ID
```js
db.users.findOne({ _id: ObjectId("$1") })
```

## 8. Поиск юзеров
```js
db.users.find({
  $and: [
    $1 ? { login: { $eq: "$1" } } : {},
    $2 ? { firstName: { $regex: "$2", $options: "i" } } : {},
    $3 ? { lastName: { $regex: "$3", $options: "i" } } : {}
  ]
})
```

## 9. Проверка существования юзера
```js
db.users.findOne({ _id: ObjectId("$1") }, { _id: 1 })
```

## 10. Создание задачи
```js
db.tasks.insertOne({
  goalId: ObjectId("$1"),
  title: "$2",
  status: "todo",
  createdAt: new Date()
})
```

## 11. Получение задачи по ID
```js
db.tasks.findOne({ _id: ObjectId("$1") })
```

## 12. Все задачи по ID цели
```js
db.tasks.find({ goalId: ObjectId("$1") })
        .sort({ createdAt: 1 })
        .toArray()
```

## 13. Обновление статуса задачи
```js
db.tasks.updateOne(
  { _id: ObjectId("$1"), goalId: ObjectId("$2") },
  { $set: { status: "$3" } }
)
```