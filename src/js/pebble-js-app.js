Pebble.addEventListener("showConfiguration",
  function(e) {
    //Load the remote config page
    Pebble.openURL("http://winterwinter.github.io/pokemonpebble/");
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var tempScale = JSON.parse(decodeURIComponent(e.response));
    //console.log("Configuration window returned: " + JSON.stringify(tempScale));
    
    var dictionary = {
      "KEY_POKEMON" : tempScale.pokemon,
      "KEY_TICKS" : tempScale.ticks,
      "KEY_SCALE" : tempScale.scale,
      "KEY_HANDS" : tempScale.hands,    

       };

    //Send to Pebble, persist there
    Pebble.sendAppMessage(dictionary,
      function(e) {
        //console.log("Sending settings data...");
      },
      function(e) {
        //console.log("Settings feedback failed!");
      }
    );
  }
);

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(text) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(text);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp);
      //console.log("Temperature is " + temperature);

      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          //console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          //console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  //console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    //console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    //console.log("AppMessage received!");
    getWeather();
  }                     
);