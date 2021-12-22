import { initializeApp } from 'firebase/app'
import {
    getFirestore, collection, onSnapshot,
    addDoc, deleteDoc, doc,
    query, where, orderBy, serverTimestamp, limit
} from 'firebase/firestore'

import axios from 'axios'

const firebaseConfig = {
    apiKey: "xxxxxxxxxxxxxxxxxxxxxxxxxxx",
    authDomain: "xxxxxxxxxxxxxxxxxxxxxxxxxxx",
    projectId: "xxxxxxxxxxxxxxxxxxxxxxxxxxx",
    storageBucket: "xxxxxxxxxxxxxxxxxxxxxxxxxxx",
    messagingSenderId: "xxxxxxxxxxxxxxxxxxxxxxxxxxx",
    appId: "xxxxxxxxxxxxxxxxxxxxxxxxxxx",
    measurementId: "xxxxxxxxxxxxxxxxxxxxxxxxxxx"
};

// init firebase
initializeApp(firebaseConfig)

// init services
const db = getFirestore()

// collection ref
const colRef = collection(db, 'users/UID/Sensors/Location/Data')

// queries
const q = query(colRef, orderBy('Time', 'desc'), limit(1))

// realtime collection data
onSnapshot(q, (snapshot) => {
    let books = []
    snapshot.docs.forEach(doc => {
        books.push({ ...doc.data(), id: doc.id })
        console.log(doc.data().Temp)
        console.log(doc.data().Humidity)
        document.getElementById("temp").innerHTML = doc.data().Temp;
        document.getElementById("humidity").innerHTML = doc.data().Humidity;
        if (doc.data().Temp >= 26) {
            document.getElementById("status").innerHTML = "Hot";
        }
        if (doc.data().Temp <= 19) {
            document.getElementById("status").innerHTML = "Cold";
        }
        if (doc.data().Temp > 19 && doc.data().Temp < 26) {
            document.getElementById("status").innerHTML = "Good";
        }
    })
    console.log(books)
})

document.getElementById("demo").addEventListener("click", getApidata);

function getApidata() {
    const axios = require('axios');

    axios.get('https://api.nasa.gov/planetary/apod?api_key=DEMO_KEY')
        .then(response => {
            document.getElementById("imageId").src = response.data.url;
            document.getElementById("imageId").alt = response.data.explanation;
        })
        .catch(error => {
            console.log(error);
        });
}