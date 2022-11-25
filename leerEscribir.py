import pyrebase

config = {
    "apiKey": "AIzaSyDQOJHIMB9CDaP8ULcW3Xvv2oCR_hFq7p8",
    "authDomain": "proyecto1-4fb45.firebaseapp.com",
    "projectId": "proyecto1-4fb45",
    "storageBucket": "proyecto1-4fb45.appspot.com",
    "messagingSenderId": "934029055643",
    "appId": "1:934029055643:web:54f813216b92f9ba87983e",
    "measurementId": "G-9BBL25KP3Q",
    "databaseURL": "https://proyecto1-4fb45-default-rtdb.firebaseio.com/",
}

firebase = pyrebase.initialize_app(config)
db = firebase.database()

all_sensors=db.child("test").get()
for sensor in all_sensors.each():
    if(sensor.key() == "numero"):
        if(int(sensor.val())>9):
            db.child("test").update({"usuario":"Elemento mayor a 9"})

all_updated = db.child("test").get()
for sensor in all_updated.each():
    print(sensor.key() + ":", sensor.val())