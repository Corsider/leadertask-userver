db.createCollection("goals", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["title", "userId"],
      properties: {
        title: {
          bsonType: "string",
          description: "Обязательное название цели",
          minLength: 1,
          maxLength: 200
        },
        description: {
          bsonType: ["string", "null"],
          maxLength: 2000
        },
        userId: {
          bsonType: "int",
          minimum: 1
        }
      },
      additionalProperties: false
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

try {
  db.goals.insertOne({ title: 123 });
} catch(e) { print("OK: " + e.errmsg); }

try {
  db.goals.insertOne({ title: "X", userId: "abc" });
} catch(e) { print("OK: " + e.errmsg); }

try {
  db.goals.insertOne({ title: "", userId: 1 });
} catch(e) { print("OK: " + e.errmsg); }