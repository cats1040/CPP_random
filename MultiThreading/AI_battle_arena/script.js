function sendPrompt() {
  const prompt = document.getElementById('prompt').value;

  fetch('http://localhost:8080/submit', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: `prompt=${encodeURIComponent(prompt)}`,
  })
    .then(response => {
      if (!response.ok) {
        throw new Error(`HTTP error! Status: ${response.status}`);
      }
      return response.text();
    })
    .then(data => {
      document.getElementById('response').textContent = "Server Response: " + data;
    })
    .catch(error => console.error('Error:', error));
}

document.getElementById('submit').addEventListener('click', sendPrompt);
