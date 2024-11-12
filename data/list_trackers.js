document.addEventListener("DOMContentLoaded", function() {
    fetch('/tracker_list_api')
        .then(response => response.json())
        .then(data => populateTable(data))
        .catch(error => console.error('Error fetching data:', error));
});

function populateTable(data) {
    const tableBody = document.querySelector('#trackerTable tbody');
    data.forEach(item => {
        const row = document.createElement('tr');

        const idCell = document.createElement('td');
        idCell.textContent = item.id;
        row.appendChild(idCell);

        const timeCell = document.createElement('td');
        timeCell.textContent = item.age;
        timeCell.classList.add('time-column');
        row.appendChild(timeCell);

        const latitudeCell = document.createElement('td');
        latitudeCell.textContent = item.latitude;
        row.appendChild(latitudeCell);

        const longitudeCell = document.createElement('td');
        longitudeCell.textContent = item.longitude;
        row.appendChild(longitudeCell);

        const altitudeCell = document.createElement('td');
        altitudeCell.textContent = item.altitude;
        altitudeCell.classList.add('altitude-column');
        row.appendChild(altitudeCell);

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

        tableBody.appendChild(row);
    });
}
