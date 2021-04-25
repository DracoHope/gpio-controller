// Please read the following instruction for installing the require module for this application
// **************************************************************************************
// Pre-requisite for executing this project
// 1. Please install NodeJS. Please select the latest version of the LTS version
//    https://nodejs.org/en/ 
// 2. Please install MongoDB for local database testing and further development.
//    Select the MongoDB Community version
//    https://www.mongodb.com/try/download/community
//    Please take note of the MongoDB installed directiory. This is require to start the MongoDB Database from the Git Bash Terminal
//    The default installed directory will look something like this: C:\Program Files\MongoDB\Server\4.4\
//    The number "4.4" will change according to the MongoDB Database Version which is been installed
// 3. Create a new folder and name this folder as "data" in the C drive
//    This will be data folder for the MongoDB Database.
// 4. Please install Git Bash
//    Git Bash is require to start the MongoDB Database
//    https://git-scm.com/downloads
//    Create a file name it as ".bash_profile" in the Git Bash Root Dictory
//    copy and insert the following script into the Git Bash ".bash_profile" config file
//    alias mongod="/c/Program\ files/MongoDB/Server/4.4/bin/mongod.exe"
//    alias mongo="/c/Program\ Files/MongoDB/Server/4.4/bin/mongo.exe"
//    Please change the MongoDB installed directory and change it accordingly to match your MongoDB installtion dictory
//    You may also just copy this ".bash_profile" file from this project folder and paste this file into your Git Bash Root Dictory
// 5. You may also download and install the Robo 3T. This is the GUI application to visualize and accessing the MongoDB Database collections and document
//    https://studio3t.com/download/?source=robomongo&medium=homepage
// **************************************************************************************
// Please install the require node module before executing this application. 
// Please follow the following step:
// Step 1: Open this project with a IDE such as Visual Studio Code
// Step 2: If using Visual Studio Code. Open a new Terminal by using the "Terminal" tag at the Menu Bar
//         This should open a Terminal with the exact workspace path for this project
// Step 3: If using other IDE, please ensure the Terminal also pointing to this project folder 
// Step 4: Install the require node module by typing in the Termoinal: 
//         npm install
//         This should start install the require node module and an additional folder name "node_modules" will be created in gthis project folder
//         All require node module stated in the "package.json" file will be install accordingly
// Step 5: If there is any vulnerability warning after executing the <npm install>, then just execute the <npm update>. 
//         npm update
//         This will update the node module to the laest version for any vulnerability warning
// **************************************************************************************
// For Creating new project please follow the instruction below:
// Installation of require packages
// Step 1: npm init -y
// Step 2: npm i express
// Step 3: npm i body-parser
// Step 4. npm i request
// Step 5: npm install --save-dev nodemon
// or npm i express body-parser ejs mongoose lodash request
// **************************************************************************************
// Express Server Setup
const https = require('https');
const request = require('request'); // Require for sending a Http request

const express = require("express");
const bodyparser = require("body-parser");  // Require for parsering the html body content by POST request
const querystring = require('querystring'); // Require for parsering the query string content by GET request
const ejs = require("ejs");                 // Passing and processing of variable from backend to frontend
const { Server } = require("http");

// npm i mongoose
const mongoose = require("mongoose");       // Require for MongoDB

// npm i lodash
const _ = require("lodash");                // Require for resolving case sensitive issue for uRL

// Requiring a own customize module for generating the "day" for the Web Page
// Our own module is not by installation so we must specify the full path in order to access it
const date = require(__dirname + "/date.js");

const favicon = require('serve-favicon');

const app = express();


// *********************** Procedure to Setup the MOngoose Database **************************
// Step: 1
// Connect to Mongo Server and create a database by the name "todolistDB" if is not exist
// "{useNewUrlParser: true}" add this parameter to avoid the deprecation warning
//
// Connecting to MongoDB Local Database. The default Port Number for MongoDB Database is "27017"
// We can connect to an existing MongoDB Database or create a new Database if is not exist
// The below connection string will be connectin to the MongoDB at "mongodb://localhost:27017/" 
// The connecting Database name is "esp32DB"
// If "esp32DB" is not exist then it will create a new Database by this name. You may change the database name to your own preference 
mongoose.connect("mongodb://localhost:27017/esp32DB", {useNewUrlParser: true, useUnifiedTopology: true});
//
// Connecting to remote MongoDB Atlas Database
// Please create a user account at: https://www.mongodb.com/cloud/atlas/register
// mongoose.connect("mongodb+srv://<user name:<password>@cluster0.<Cluster Name>.mongodb.net>/<Database Name>", {useNewUrlParser: true, useUnifiedTopology: true});

// Step: 2A
// Create a new Schema for the Mongo Server Database
// Basically the Button Object Blueprint
const buttonsSchema = {
  _id: String,
  gpio: String,
  color: String,
  label: String,
  status: String
   };

// Step 3A: Creating a MongoDB Model for the buttonsSchema
// This will create a "Buttons" object which will representing individual user customized button been created. 
const Button = mongoose.model("Button", buttonsSchema);

// Step: 2A
// Create a new Schema for the Mongo Server Database
// Basically the UserButton Object Blueprint
// Please take note the UserButton Object contain an Array of Button Object created from Step 2A and 2B
const userButtonSchema = {
  // property/Field Name : Field Data Type
  _id: String,
  userURL: String,
  applicationPort: String,
  // An Array of Button Object creating from Step 2A and Step 2B
  buttons : [buttonsSchema]
};

// Step: 3B
// Finally Create the Model of the "userButton" object
// This step will also create the "collection/table" in the Database
// The model name first letter must be capitalise
// "mongoose.model("Item", itemsSchema)"
// The first parameter must be the singular version of the collection Name
// This mean if we enter "item" then the collection name in the database is "items"
// The second parameter is the Schema which we have created in Step 2 then
// the collection/table will be created base on this Schema
const userButton = mongoose.model("userButton", userButtonSchema);

// **************************************************************************************
// All html, images, css, javascript files are all consider static files
// In order Express Server to use these static files we must create a new folder to contain them all
// The below syntax is to tell Express Server the location of our static files all in this "public" folder
// This will all Express Server to provide a relative path to these static files
app.use(express.static("public"));
app.set('view engine', 'ejs');
// Add favicon icon to the frontend web page
app.use(favicon(__dirname + '/public/images/smart-home.ico'));

app.use(bodyparser.urlencoded({extended: true}));

// **************************************************************************************
// Declaring Variable
let api = ""
let sensorName = "";
let sensorLocation = "";
let counter = "0";
let temperature = "0";
let humidity = "0";
let heatIndex = "0";

let day = date.getDate();

// This is a dynamic base URL created by the User to access the devices
let webServerURL = ""

// URL and Port Configuration
baseURL = "";
appPort = "";

// **************************************************************************************
// This is the starting Root route of this application
// This will be using the default "Demo" account in the MongoDB Database
app.get("/", function(req, res){
  // console.log("In the Root Route Displaying Demo ESP32 GPIO Controller Dashboard");

  // We want to always capitalise the first letter of the Student Name
  // We be using the "lodash" - https://lodash.com/docs/#capitalize
  // Example: _.capitalize('FRED'); => 'Fred'
  const studentName = _.capitalize("Demo");

  // Please take note: "findOne()" method will only return only one documents
  // but "find()" method will return an Array of documents
  // Therefore we cannot check the length for the result of "findOne()"
  // but we can "if (!foundList)" to check whether is there any result been found
  // ----------------------------------------------------------------------------
  userButton.findOne({_id: studentName}, function(err, foundList){
    if (!err){
      // If no existing document/record found for this customListName
      // Then we will populate it with the default list of items
      if (!foundList){
        //Create a new list of todolist items with the ":customListName"
        // Each item is consider one document
        const user = new userButton({
          _id: studentName,
          userURL: '',
          applicationPort: '',
          buttons: [ ]
        });
        user.save();
        console.log(`Demo Record Name: ${studentName} been added into Database`)

        res.render("ESP32_User_Auto_Generate_Button", {studentName: studentName, elementArray: user.buttons, webServerURLBaseURL: user.userURL, baseURL:user.userURL, appPort: user.applicationPort });
      } else {
        res.render("ESP32_User_Auto_Generate_Button", {studentName: foundList._id, elementArray: foundList.buttons, webServerURLBaseURL: foundList.userURL, baseURL:foundList.userURL, appPort: foundList.applicationPort });
      }
    }
  });
});
  
// **************************************************************************************
// Control ESP32 Module - This is for configuring the Base URL for the device to be access
// Ngork URL Enable - When using Ngork URL or other Domain URL then the Application Port number
// will be optional according to the user requirement
app.get("/ip", function(req, res){
  console.log("This is the /ip route which is for configuring the Base URL for the device to be access");
  console.log(req.query);
  ipAddress = req.query.ipAddress;
  port = req.query.port;
  route = req.query.route;

  // Display Current Configured Base URL and Port
  baseURL = ipAddress;
  appPort = port;

  if(baseURL == ""){
    baseURL = "Please Enter The Base URL for your Device.";
  }

  if(appPort == ""){
    appPort = "No Application Port Number";
  }

  if(port != '') {
    webServerURL = "http://" + ipAddress + ":" + port + "/";
  } else {
    webServerURL = ipAddress + "/";
  }

  console.log("********* The Base URL is: " + webServerURL)
  res.redirect("/hardCodedButton");
});

// **************************************************************************************
// ledControl
// This is the actual route for making the request to the remote devices with the Base URL/condition
// Base URL: webServerURL
// Condition: led + "/" + ledStatus where led is referring to the GPIO number and 
// the ledStatus is the state of this GPIO either on or off
// The sample URL for this route is as folllow:
// http://localhost:3000/esp32-controller?led=26&ledStatus=on
app.post("/gpio/ledControl", function(req, res){
  console.log("This is the ledControl Route - GET Request Data received: ");
  // *********************************************
  console.log(req.body);
  // led = req.query.led;
  // ledStatus = req.query.ledStatus;
  // const esp32BaseURL = req.query.esp32BaseURL
  led = req.body.led;
  ledStatus = req.body.ledStatus;
  const esp32BasePort = req.body.esp32BasePort;
  console.log(`The Port is: ${esp32BasePort}`)
  let esp32BaseURL = ""
  if(esp32BasePort != "No")
  {
    esp32BaseURL = "http://" + req.body.esp32BaseURL  + ":" + esp32BasePort + "/"
  } else { 
    esp32BaseURL =  req.body.esp32BaseURL + "/"
  }

  const url = esp32BaseURL + led + "/" + ledStatus;
  console.log(`The Base URL is ${esp32BaseURL}`)
  console.log("********* Ming ESP LED URL: " + url)

  // request(url, { json: true }, (err, res, body) => {
    request(url, (err, res, body) => {
    if (err) { return console.log(err); }
    console.log(body.url);
    console.log(body.explanation);
    })

    res.render("ESP32_Hardcoded_Button", {webServerURLBaseURL: "esp32BaseURL"});
  
  });

// **************************************************************************************
// ESP32 Controller Dashboard
// This is the route for getting the updated data for the ESP32 Dashboard
app.get("/ESP32_Display", function(req, res){
  console.log("This is the GET Root Route request Data received: ");
  console.log(req.query)
  console.log("********* Ming API: " + api)
  today = date.getDate();
 
  console.log(`####################### Ming in the '/' Root Route - The Temperature is: ${temperature}  The Humidity is: ${humidity}`)
  
  res.render("ESP32_Display", {api_key: api, tdy_date:today, sensorName: sensorName, sensorLocation: sensorLocation, counter:counter, temperature : temperature, humidity : humidity, heatIndex : heatIndex});
  //res.sendStatus(200);
});

// **************************************************************************************
// This is the Data updating route for the Arduino ESP32 Device
app.post("/data", function(req, res){
  //console.log("This is the POST request Data received: " + req.body);
  //console.log(req.body);
  let key = req.body.api_key

  sensorName = req.body.sensorName;
  sensorLocation = req.body.sensorLocation;
  counter = req.body.counter;
  api = req.body.api_key;
  temperature = req.body.temperature + "°C";
  humidity = req.body.humidity + "%";
  heatIndex = req.body.heatIndex + "°C";
  app.locals.global_value = api;

  res.sendStatus(200);
});

// **************************************************************************************
// http://localhost:3000/esp32-controller?led=26&ledStatus=on
app.get("/:baseURL/:control/:status", function(req, res){
  console.log("****************** In the /:baseURL/:control/:status Sub route ******************")
  console.log(req.params.control);
  console.log(req.params.status);
  console.log(req.params.baseURL);

  let led = req.params.control;
  let ledStatus = req.params.status;
  let baseURL = req.params.baseURL;

  const url = "http://" + baseURL + "/" + led + "/" + ledStatus;
  console.log(`The Base URL is ${baseURL}`)
  console.log("********* Ming ESP LED URL: " + url)

  request(url, (err, res, body) => {
  if (err) { return console.log(err); }
    console.log(body.url);
    console.log(body.explanation);
  })

  res.send(200);

});

// **************************************************************************************

app.get("/hardCodedButton", function(req, res){
  console.log("****************** In the hardCodedButton route ******************")
  res.render("ESP32_Hardcoded_Button", {webServerURLBaseURL: webServerURL});
});

// **************************************************************************************
// ************** Creating a Student Sub-Route **************
app.get("/user/:studentName", function(req, res){
  console.log("In the dedicated Student Sub Route /:studentName");
  console.log("The /:customListName Route is: " + req.params.studentName);

  const studentName = _.capitalize(req.params.studentName);

  userButton.findOne({_id: studentName}, function(err, foundList){
    if (!err){
      // If no existing document/record found for this "studentName"
      // Then we will create a user account for this "studentName"
      if (!foundList){
        const user = new userButton({
          _id: studentName,
          userURL: '',
          applicationPort: '',
          buttons: [ ]
        });
        user.save();

        console.log(`Student Record Name: ${studentName} been added into Database`)
        res.render("ESP32_User_Auto_Generate_Button", {studentName: studentName, elementArray: user.buttons, webServerURLBaseURL: user.userURL, baseURL:user.userURL, appPort: user.applicationPort });
      } else {

        res.render("ESP32_User_Auto_Generate_Button", {studentName: foundList._id, elementArray: foundList.buttons, webServerURLBaseURL: foundList.userURL, baseURL:foundList.userURL, appPort: foundList.applicationPort });
      }
    }
  });
});

// **************************************************************************************
// This is the Button adding route. It will add the customize user button into this specific user account 
app.post("/add", function(req, res){
  console.log(req.body);
  const studentName = req.body.studentName;
  // "req.body.list" is actually the date of the to do item been entered
  const color = req.body.color;
  const gpio = req.body.gpio; 
  const gpioId = "gpio " + gpio;  // Formating the gpioId for this button element

  // Always Append the word "On" during initial Creation of button
  // The word "On" will be toggling to "Off" when sending a GPIO request to their devices
  const buttonLabel = req.body.buttonLabel + " On";

  console.log(req.body);

  const newButton = new Button({
        _id: gpioId,
        gpio: gpio,
        color: color,
        label: buttonLabel,
        status: 'on'
  });

    userButton.findOne({_id: studentName}, function(err, foundList){
    foundList.buttons.push(newButton);
    foundList.save();
    const studentRoute = "/user/" + studentName;
    // console.log(studentRoute);
    
    res.redirect(studentRoute);
    });
});

// **************************************************************************************

// ************** POST request to Handle a "Delete" operation request **************
// Refer to lecture 368
app.post("/delete", function(req, res){
  console.log("****************** In the Delete Button route ******************")
  console.log(req.body);
  const gpioId = req.body.gpioId;
  const studentName = req.body.studentName;
  
  userButton.findOneAndUpdate({_id: studentName}, {$pull: {buttons: {_id: gpioId}}}, function(err, foundList){
    if (!err){
      res.redirect("/user/" + studentName);
    }
  });
});

// **************************************************************************************
app.post("/gpioRequest", function(req, res){
  try{
      console.log("****************** In the gpioRequest route ******************")
      console.log(req.body);
      studentName =  req.body.studentName;
      userBaseURL = req.body.webServerURLBaseURL;
      gpio = req.body.gpio;
      id = req.body._id;
      status = req.body.status;
      console.log(`The GPIO is: ${gpio} and _id is: ${id} `);

      url = userBaseURL + gpio + "/" + status;
      console.log(`The GPIO Request Address is: ${url} `)

        request(url, (err, body) => {
        if (err) {      
            console.log("In the /gpioRequest Route. Error Sending GPIO Request")
            console.log(err);  
          }
          else {
            userButton.findOne({_id: studentName}, function(err, foundList){
              if (!err){
                try {
                  myObj = foundList.buttons.find(obj => obj._id === id);
                  console.log(typeof(myObj))
                  console.log(myObj._id)
                  console.log(myObj.status)
                  if(myObj.status === 'on'){
                        myObj.status = 'off';
                        
                        tempLabel = myObj.label.substring(0, myObj.label.length-2) ; 
                        myObj.label = tempLabel + " Off";
                        foundList.save();
                      } else {
                        myObj.status = 'on';
                        
                        tempLabel = myObj.label.substring(0, myObj.label.length-3) ; 
                        myObj.label = tempLabel + " " + myObj.status;
                        myObj.label = tempLabel + " On";
                        foundList.save();
                      }
                  } catch {
                    console.log("In the /gpioRequest Route, Error in updating of document")
                  }
                  } else {
                    console.log("In the /gpioRequest Route, Error in updating of document")
                  }
            });
          }
        });
    } catch(err) {
      console.log("Exception catch at gpioRequest Route")
      console.log(err.message);
      studentName =  req.body.studentName;
      res.redirect("/user/" + studentName);
    } finally {
      studentName =  req.body.studentName;
      res.redirect("/user/" + studentName);
    }
  });

// **************************************************************************************
// Control ESP32 Module - This is for configuring the Base URL for the device to be access
// Ngork URL Enable - When using Ngork URL or other Domain URL then the Application Port number
// will be optional according to the user requirement
app.get("/userURL", function(req, res){
  console.log("This is the /userURL route which is for configuring the Base URL for individual user");
  console.log(req.query);
  ipAddress = req.query.ipAddress;
  port = req.query.port;
  studentName = req.query.route;

  // Display Current Configured Base URL and Port
  baseURL = ipAddress;
  appPort = port;

  if(baseURL == ""){
    baseURL = "Please Enter The Base URL for your Device.";
  }

  if(appPort == ""){
    appPort = "No Application Port Number";
  }

  if(port != '') {
    webServerURL = "http://" + ipAddress + ":" + port + "/";
  } else {
    webServerURL = ipAddress + "/";
  }

  console.log("********* The Base URL is: " + webServerURL)

  userButton.findOne({_id: studentName}, function(err, foundList){
    if (!err){
      if (!foundList){
        // If no existing document/record found for this "studentName"
        // Then we will create this User Account for this "studentName"
        const user = new userButton({
        _id: studentName,
        userURL: webServerURL,
        applicationPort: appPort,
        buttons: [ ]
        });

        webServerURL = ""
        user.save();

        console.log(`Student Record Name: ${studentName} been added into Database`)

        res.render("ESP32_User_Auto_Generate_Button", {studentName: studentName, elementArray: user.buttons, webServerURLBaseURL: user.userURL, baseURL:user.userURL, appPort: user.appPort });
        } else {
          foundList.userURL = webServerURL;
          foundList.applicationPort = appPort;
          foundList.save();
          webServerURL = "";
          appPort = "";

          res.render("ESP32_User_Auto_Generate_Button", {studentName: foundList._id, elementArray: foundList.buttons, webServerURLBaseURL: foundList.userURL, baseURL:foundList.userURL, appPort: foundList.applicationPort});
        }
    }
  });
});

// **************************************************************************************
app.get("/demoUserCreatedButton", function(req, res){
  console.log("In the Demo User Create Button Route /:studentName");
  const studentName = _.capitalize("Demo");

  userButton.findOne({_id: studentName}, function(err, foundList){
    if (!err){
      if (!foundList){
          const user = new userButton({
          _id: studentName,
          userURL: '',
          applicationPort: '',
          buttons: [ ]
        });

        user.save();

        console.log(`Demo Record Name: ${studentName} been added into Database`)
        res.render("ESP32_User_Auto_Generate_Button", {studentName: studentName, elementArray: user.buttons, webServerURLBaseURL: user.userURL, baseURL:user.userURL, appPort: user.applicationPort });
      } else {
        res.render("ESP32_User_Auto_Generate_Button", {studentName: foundList._id, elementArray: foundList.buttons, webServerURLBaseURL: foundList.userURL, baseURL:foundList.userURL, appPort: foundList.applicationPort });
      }
    }
  });
});

// **************************************************************************************
var port = process.env.PORT || 2000;

app.listen(port, function () {
  console.log('******* ESP32 GPIO Controller Express Server App listening on port ' + port + ' with Ngork URL Enable ************');
});

// **************************************************************************************
