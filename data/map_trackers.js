var map_loaded = false;
var map;

// Run initial check
checkConnectionAndLoadLeaflet();

// Function to display a warning popup
function showWarningPopup(message) {
    const popup = document.createElement('div');
    popup.id = 'warning-popup';
    popup.style.position = 'fixed';
    popup.style.top = '50%';
    popup.style.left = '50%';
    popup.style.transform = 'translate(-50%, -50%)';
    popup.style.padding = '20px';
    popup.style.backgroundColor = '#ffdddd';
    popup.style.border = '1px solid #ff0000';
    popup.style.borderRadius = '8px';
    popup.style.color = '#ff0000';
    popup.style.fontSize = '16px';
    popup.style.textAlign = 'center';
    popup.style.zIndex = '1000';

    // Create warning message text
    const messageText = document.createElement('p');
    messageText.innerText = message;
    messageText.style.marginBottom = '15px';
	

    // Create close button
    const closeButton = document.createElement('button');
    closeButton.innerText = 'Close';
    closeButton.style.padding = '15px 20px';
    closeButton.style.border = 'none';
    closeButton.style.backgroundColor = '#ff8888';
    closeButton.style.color = '#fff';
    closeButton.style.cursor = 'pointer';
    closeButton.style.borderRadius = '4px';
	closeButton.style.width = '100%';
    closeButton.onclick = () => document.body.removeChild(popup);

    // Append message and button to popup
    popup.appendChild(messageText);
    popup.appendChild(closeButton);
    document.body.appendChild(popup);
}

// Function to load Leaflet CSS dynamically
function loadLeafletCSS() {
    const link = document.createElement('link');
    link.rel = 'stylesheet';
    link.href = "https://unpkg.com/leaflet@1.9.4/dist/leaflet.css";
    link.integrity = "sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=";
    link.crossOrigin = "";
    link.onload = () => console.log("Leaflet CSS loaded successfully.");
    link.onerror = () => console.log("Failed to load Leaflet CSS.");
    document.head.appendChild(link);
}

// Function to load Leaflet JS dynamically
function loadLeafletJS() {
    const script = document.createElement('script');
    script.src = "https://unpkg.com/leaflet@1.9.4/dist/leaflet.js";
    script.integrity = "sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=";
    script.crossOrigin = "";
    script.onload = () => {
        console.log("Leaflet JS loaded successfully.");
        displayMap();
        updateTrackersPosition();
    };
    script.onerror = () => console.log("Failed to load Leaflet JS.");
    document.head.appendChild(script);
}

function loadMap(){
	if(map_loaded == false){
		map_loaded = true;
		loadLeafletCSS();
		loadLeafletJS();
	}
}

// Function to check and load Leaflet CSS and JS if online
function checkConnectionAndLoadLeaflet() {
    if (navigator.onLine) {
        console.log("Internet connection available.");
        loadMap();
    } else {
        console.log("No internet connection. Leaflet will not be loaded.");
		showWarningPopup("No internet connection. Map cannot be loaded.");
    }
}

// Display map once Leaflet is loaded
function displayMap() {
    // Check for Geolocation API support
    if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(
            (position) => {
                const userLat = position.coords.latitude;
                const userLng = position.coords.longitude;

                // Initialize the map at user's current location
                map = L.map('map').setView([userLat, userLng], 13);

                // Set up the tile layer from OpenStreetMap
                L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                    maxZoom: 19,
                    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
                }).addTo(map);

                // Add a marker at user's location
                L.marker([userLat, userLng]).addTo(map)
                    .bindPopup('You are here');
            },
            (error) => {
                console.log("Unable to retrieve your location", error);
                // Default location if permission denied or unavailable
                // Initialize the map at user's current location
                const map = L.map('map').setView([50.0, 20.0], 5);

                // Set up the tile layer from OpenStreetMap
                L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                    maxZoom: 19,
                    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
                }).addTo(map);
            }
        );
    } else {
        console.log("Geolocation is not supported by this browser.");
        // Default location if Geolocation API not available
        initializeDefaultMap();
    }
}

function updateTrackersPosition() {
	fetch('/tracker_list_api')
        .then(response => response.json())
        .then(data => populateMap(data))
        .catch(error => console.error('Error fetching data:', error));
}

function populateMap(data) {
	if(data.length == 0) 
		return;
	
	var markersLayer = L.layerGroup();
	
	data.forEach(item => {
		/*
		item.id
		item.age
		item.latitude
		item.longitude
		item.altitude
		
		const mapCell = document.createElement('td');
        const mapButton = document.createElement('button');
        mapButton.textContent = 'Go to Map';
        mapButton.onclick = (event) => {
            event.stopPropagation();
            const mapUrl = `https://www.google.com/maps?q=${item.latitude},${item.longitude}`;
            window.open(mapUrl, '_blank');
        };
        mapCell.classList.add('map-column');
        mapCell.appendChild(mapButton);
        row.appendChild(mapCell);

        row.onclick = () => {
            const mapUrl = `https://www.google.com/maps?q=${item.latitude},${item.longitude}`;
            window.open(mapUrl, '_blank');
        };
		
		
		*/
		
		// Check if latitude and longitude are not null
		if (item.latitude !== null && item.longitude !== null) {
            if(item.type == 'own_position'){
                // Add marker to the map at the given latitude and longitude
                const marker = L.marker([item.latitude, item.longitude], {icon: greenIcon});
                // Add a popup with flight details to the marker
                marker.bindPopup(createPopupContent(item.id, item.latitude, item.longitude, item.altitude, item.age));
                markersLayer.addLayer(marker);
            } else {
                // Add marker to the map at the given latitude and longitude
                const marker = L.marker([item.latitude, item.longitude]);
                // Add a popup with flight details to the marker
                marker.bindPopup(createPopupContent(item.id, item.latitude, item.longitude, item.altitude, item.age));
                markersLayer.addLayer(marker);
            }
			
		}	
	});
	
	map.addLayer(markersLayer);
}

// Function to create the content for the popup
function createPopupContent(objectId, flightId, lastLatitude, lastLongitude, lastAltitude, timeDifference) {
    const popupContent = document.createElement('div');

    const infoElement = document.createElement('p');
	infoElement.innerHTML = `Object ID: ${objectId}<br>Last Latitude: ${lastLatitude}<br>Last Longitude: ${lastLongitude}<br>Last Altitude: ${lastAltitude}<br>Data Age: ${timeDifference}`;
    popupContent.appendChild(infoElement);

    return popupContent;
}