import requests
import random
import string
import sys

BASE_URL = "http://localhost:8080"


def random_string(n=8):
    return ''.join(random.choices(string.ascii_lowercase, k=n))


def print_result(name, ok, response=None):
    if ok:
        print(f"[OK] {name}")
    else:
        print(f"[FAIL] {name}")
        if response is not None:
            print("Status:", response.status_code)
            try:
                print("Body:", response.json())
            except:
                print("Body:", response.text)
        sys.exit(1)


# TEST 1: Registration
def test_registration():
    login = f"user_{random_string()}"
    password = "12345678"
    first_name = "John"
    last_name = "Doe"

    response = requests.post(
        f"{BASE_URL}/api/users",
        json={
            "login": login,
            "password": password,
            "firstName": first_name,
            "lastName": last_name,
        },
    )

    ok = response.status_code == 200
    print_result("User registration", ok, response)

    return login, password


# TEST 2: Login (get JWT)
def test_login(login, password):
    response = requests.post(
        f"{BASE_URL}/api/users/login",
        json={
            "login": login,
            "password": password,
        },
    )

    ok = response.status_code == 200
    print_result("User login", ok, response)

    data = response.json()
    token = data.get("accessToken")

    if not token:
        print("[FAIL] No token returned")
        sys.exit(1)

    print("[OK] JWT received")
    return token

def test_login_bad_password(login, password):
    response = requests.post(
        f"{BASE_URL}/api/users/login",
        json={
            "login": login,
            "password": password + "1",
        },
    )

    ok = response.status_code == 401
    print_result("User cannot login", ok, response)

    data = response.json()
    token = data.get("accessToken")

    if token:
        print("[FAIL] Token returned")
        sys.exit(1)

    print("[OK] No token returned")
    return token


# TEST 3: Search users
def test_search(login, token):
    headers = {
        "Authorization": f"Bearer {token}"
    }

    response = requests.get(
        f"{BASE_URL}/api/users/search",
        params={"login": login},
        headers=headers,
    )

    ok = response.status_code == 200
    print_result("User search", ok, response)

    data = response.json()

    if not isinstance(data, list) or len(data) == 0:
        print("[FAIL] Search returned empty result")
        sys.exit(1)

    found = any(user["login"] == login for user in data)
    print_result("User found in search", found, response)

# GOALS
def test_create_goal(token):
    response = requests.post(
        f"{BASE_URL}/api/goals",
        json={
            "title": f"Goal {random_string()}",
            "description": "Test goal"
        },
        headers={"Authorization": f"Bearer {token}"},
    )

    print_result("Create goal", response.status_code == 201, response)

    goal_id = response.json().get("id")
    if not goal_id:
        print("[FAIL] No goal id")
        sys.exit(1)

    return goal_id


def test_get_goals(token, goal_id):
    response = requests.get(
        f"{BASE_URL}/api/goals",
        headers={"Authorization": f"Bearer {token}"},
    )

    print_result("Get goals", response.status_code == 200, response)

    data = response.json()
    found = any(g["id"] == goal_id for g in data)
    print_result("Goal exists in list", found, response)


# TASKS
def test_create_task(token, goal_id):
    response = requests.post(
        f"{BASE_URL}/api/goals/{goal_id}/tasks",
        json={"title": "Test task"},
        headers={"Authorization": f"Bearer {token}"},
    )

    print_result("Create task", response.status_code == 201, response)

    task_id = response.json().get("id")
    if not task_id:
        print("[FAIL] No task id")
        sys.exit(1)

    return task_id


def test_get_tasks(token, goal_id, task_id):
    response = requests.get(
        f"{BASE_URL}/api/goals/{goal_id}/tasks",
        headers={"Authorization": f"Bearer {token}"},
    )

    print_result("Get tasks", response.status_code == 200, response)

    data = response.json()
    found = any(t["id"] == task_id for t in data)
    print_result("Task exists in list", found, response)


def test_update_task_status(token, goal_id, task_id):
    response = requests.put(
        f"{BASE_URL}/api/goals/{goal_id}/tasks/{task_id}/status",
        json={"status": "done"},
        headers={"Authorization": f"Bearer {token}"},
    )

    print_result("Update task status", response.status_code == 200, response)

    status = response.json().get("status")
    print_result("Task status updated", status == "done", response)

if __name__ == "__main__":
    print("Running tests...\n")

    login, password = test_registration()
    token = test_login(login, password)
    test_login_bad_password(login, password)
    test_search(login, token)

    # goals
    goal_id = test_create_goal(token)
    test_get_goals(token, goal_id)

    # tasks
    task_id = test_create_task(token, goal_id)
    test_get_tasks(token, goal_id, task_id)
    test_update_task_status(token, goal_id, task_id)

    print("\nALL TESTS PASSED ✅")