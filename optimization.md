# Оптимизация запросов

## Получение user по id

```
-- Получение пользователя по ID
SELECT id, login, password, first_name, last_name, created_at
FROM users
WHERE id = $1;
```

Результат EXPLAIN ANALYZE:

```
goals_db=# explain analyze SELECT id, login, password, first_name, last_name, created_at
FROM users
WHERE id = 1;
-[ RECORD 1 ]------------------------
QUERY PLAN | Index Scan using users_pkey on users  (cost=0.14..8.15 rows=1 width=2080) (actual time=0.044..0.045 rows=1 loops=1)
-[ RECORD 2 ]------------------------
QUERY PLAN |   Index Cond: (id = 1)
-[ RECORD 3 ]------------------------
QUERY PLAN | Planning Time: 0.113 ms
-[ RECORD 4 ]------------------------
QUERY PLAN | Execution Time: 0.072 ms
```

Оптимизация не требуется. Используется индекс users_pkey

## Получение всех целей юзера

```
-- Получение всех целей пользователя
SELECT id, user_id, title, description, created_at
FROM goals
WHERE user_id = $1
ORDER BY created_at DESC;
```

Результат EXPLAIN ANALYZE

```
goals_db=# explain analyze SELECT id, user_id, title, description, created_at
FROM goals
WHERE user_id = 1
ORDER BY created_at DESC;
-[ RECORD 1 ]-----------------------------
QUERY PLAN | Sort  (cost=8.17..8.18 rows=1 width=572) (actual time=0.279..0.279 rows=2 loops=1)
-[ RECORD 2 ]-----------------------------
QUERY PLAN |   Sort Key: created_at DESC
-[ RECORD 3 ]-----------------------------
QUERY PLAN |   Sort Method: quicksort  Memory: 25kB
-[ RECORD 4 ]-----------------------------
QUERY PLAN |   ->  Index Scan using idx_goals_user_id on goals  (cost=0.14..8.16 rows=1 width=572) (actual time=0.240..0.241 rows=2 loops=1)
-[ RECORD 5 ]-----------------------------
QUERY PLAN |         Index Cond: (user_id = 1)
-[ RECORD 6 ]-----------------------------
QUERY PLAN | Planning Time: 1.355 ms
-[ RECORD 7 ]-----------------------------
QUERY PLAN | Execution Time: 0.304 ms
```

Создадим индекс

```
goals_db=# create index concurrently if not exists idx_created_at on goals(created_at);
CREATE INDEX
```

Посмотрим результат EXPLAIN ANALYZE

```
goals_db=# explain analyze SELECT id, user_id, title, description, created_at
FROM goals
WHERE user_id = 1
ORDER BY created_at DESC;
-[ RECORD 1 ]----------------------------
QUERY PLAN | Sort  (cost=1.17..1.18 rows=1 width=572) (actual time=0.027..0.028 rows=2 loops=1)
-[ RECORD 2 ]----------------------------
QUERY PLAN |   Sort Key: created_at DESC
-[ RECORD 3 ]----------------------------
QUERY PLAN |   Sort Method: quicksort  Memory: 25kB
-[ RECORD 4 ]----------------------------
QUERY PLAN |   ->  Seq Scan on goals  (cost=0.00..1.16 rows=1 width=572) (actual time=0.007..0.008 rows=2 loops=1)
-[ RECORD 5 ]----------------------------
QUERY PLAN |         Filter: (user_id = 1)
-[ RECORD 6 ]----------------------------
QUERY PLAN |         Rows Removed by Filter: 11
-[ RECORD 7 ]----------------------------
QUERY PLAN | Planning Time: 0.261 ms
-[ RECORD 8 ]----------------------------
QUERY PLAN | Execution Time: 0.049 ms
```

Индекс ускорил запрос