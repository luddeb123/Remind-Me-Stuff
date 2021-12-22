const functions = require("firebase-functions");
const admin = require("firebase-admin");
admin.initializeApp();

const db = admin.firestore();

// Logs when a new user has been created.
exports.onUserCreate = functions.region("europe-west3")
    .firestore.document("users/{userId}")
    .onCreate(async (snap, context) => {
      const values = snap.data();
      const now = admin.firestore.Timestamp.now();
      await db.collection("logging")
          .add({description: `User was created: ${values.Username} at ${now}`});
    }),

// Everytime a datapoint is added from a sensor, add a timestamp to it.
exports.onSensordataAdd = functions.region("europe-west3")
    .firestore.document("users/{userId}/Sensors/{location}/Data/{datapoint}")
    .onCreate(async (snap, context) => {
      const now = admin.firestore.Timestamp.now();
      await snap.ref.update({
        Time: now});
    });
