-- Создание пользователя
INSERT INTO users (login, password, first_name, last_name)
VALUES ($1, $2, $3, $4)
RETURNING id;

-- Получение пользователя по логину
SELECT id, login, password, first_name, last_name, created_at
FROM users
WHERE login = $1;

-- Получение пользователя по ID
SELECT id, login, password, first_name, last_name, created_at
FROM users
WHERE id = $1;

-- Поиск пользователей
SELECT id, login, password, first_name, last_name, created_at
FROM users
WHERE ($1::TEXT IS NULL OR login = $1)
  AND ($2::TEXT IS NULL OR first_name LIKE '%' || $2 || '%')
  AND ($3::TEXT IS NULL OR last_name LIKE '%' || $3 || '%');

-- Проверка существования пользователя
SELECT EXISTS(SELECT 1 FROM users WHERE id = $1);

-- Создание цели
INSERT INTO goals (user_id, title, description)
VALUES ($1, $2, $3)
RETURNING id, user_id, title, description, created_at;

-- Получение цели по ID
SELECT id, user_id, title, description, created_at
FROM goals
WHERE id = $1;

-- Получение всех целей пользователя
SELECT id, user_id, title, description, created_at
FROM goals
WHERE user_id = $1
ORDER BY created_at DESC;


-- Создание задачи
INSERT INTO tasks (goal_id, title, status)
VALUES ($1, $2, 'todo')
RETURNING id, goal_id, title, status, created_at;

-- Получение задачи по ID
SELECT id, goal_id, title, status, created_at
FROM tasks
WHERE id = $1;

-- Получение всех задач по ID цели
SELECT id, goal_id, title, status, created_at
FROM tasks
WHERE goal_id = $1
ORDER BY created_at;

-- Обновление статуса задачи
UPDATE tasks
SET status = $3
WHERE id = $1 AND goal_id = $2
RETURNING id, goal_id, title, status, created_at;