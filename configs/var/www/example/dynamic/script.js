function showMessage(message, duration = 1000) {
	const status = document.getElementById("status-message");
	status.textContent = message;
	status.style.display = "block";

	// Hide the message after a few seconds
	status.classList.remove("hidden");
	setTimeout(() => {
		status.classList.add("hidden");
	}, duration);
}

// Load saved stories on page load
function fetchStoriesFromDirectory() {
  fetch("/../upload/")
  .then(response => {
    if (!response.ok) {
      throw new Error("Failed to fetch directory listing");
    }
    return response.text(); // ✅ Read the body of the response as plain text (HTML)
  })
  .then(html => {
    const parser = new DOMParser();
    const doc = parser.parseFromString(html, "text/html");
    const links = doc.querySelectorAll("a");

    const jsonFiles = Array.from(links)
      .map(link => link.getAttribute("href"))
      .filter(name => name.endsWith(".json"));

    return jsonFiles;
  })
  .then(fileNames => {
    fileNames.forEach(filename => {
      fetch(filename, {
        method: "GET",
        version: "1.1",
        headers: { "Content-Type": "application/json" }})
        .then(response => {
          if (!response.ok) {
            throw new Error("Failed to load " + filename);
          }
          return response.json();
        })
        .then(data => {
          addStoryToList(data.text, new Date(data.time), data.user);
        })
        .catch(err => console.error("Error loading story:", filename, err));
    });
  })
  .catch(err => {
    console.error("Failed to fetch or parse directory listing:", err);
  });
}

window.addEventListener("DOMContentLoaded", fetchStoriesFromDirectory);


function escapeHtml(text) {
  const div = document.createElement("div");
  div.textContent = text;
  return div.innerHTML;
}

// Function to add story to the visible list
function addStoryToList(text, date, username) {
  const li = document.createElement("li");
  li.setAttribute("data-username", username); // 👈 tag with username

  const timestampSpan = document.createElement("span");
  timestampSpan.className = "timestamp";
  timestampSpan.textContent = date.toLocaleString();

  const userSpan = document.createElement("span");
  userSpan.className = "story-user";
  userSpan.textContent = ` — by ${username}`;

  li.appendChild(document.createTextNode(escapeHtml(text) + " "));
  li.appendChild(timestampSpan);
  li.appendChild(userSpan);

  const storyList = document.getElementById("story-list");
  storyList.insertBefore(li, storyList.firstChild);
  storyList.style.display = "block";
}

document.getElementById("story-form").addEventListener("submit", function(event) {
  event.preventDefault();

  const storyText = document.getElementById("story").value.trim();
  const username = document.getElementById("username").value.trim() || "Anonymous";

  if (storyText === "") {
    showMessage("Story cannot be empty!");
    return;
  }
  if (storyText.length > 5000) {
    showMessage("Story is too long! Max 5000 characters.");
    return;
  }

  const now = new Date();

  // Step 1: POST the story to the server
  fetch("/../upload/" + username + ".json", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      text: storyText,
      user: username,
      time: now.toISOString()
    })
  })
  .then(response => {
    if (!response.ok) {
      throw new Error("Failed to save story");
    }

    // Step 2: GET the saved story back
    return fetch("/../upload/" + username + ".json", {
      method: "GET",
      version: "1.1",
      headers: { "Content-Type": "application/json" }});
  })
  .then(response => {
    if (!response.ok) {
      throw new Error("Failed to load story");
    }
    return response.json(); // ✅ parse JSON properly
  })
  .then(data => {
    addStoryToList(data.text, new Date(data.time), data.user);
    document.getElementById("story-form").reset();
    document.getElementById("username").value = "";
    showMessage("Story submitted!");
  })
  .catch(error => {
    console.error("Error submitting or loading story:", error);
    showMessage("Failed to submit story. Please try again. " + error.message);
  });
});


document.getElementById("clear-btn").addEventListener("click", () => {
  if (confirm("Are you sure you want to clear all stories?")) {
    localStorage.removeItem("stories");
    document.getElementById("story-list").innerHTML = "";
    document.getElementById("story-list").style.display = "none";
    showMessage("All stories cleared.");
  }
});

document.getElementById("delete-btn").addEventListener("click", () => {
  const username = document.getElementById("username").value.trim();
  if (!username) {
    showMessage("Please enter your username to delete your story.");
    return;
  }

  fetch("/../upload/" + username + ".json", {
    method: "DELETE",
    version: "1.1",
    headers: { "Content-Type": "application/json" }
  })
  .then(response => {
    if (!response.ok) {
      throw new Error("Failed to delete story");
    }

    // Remove only the story <li> with this username
    const storyList = document.getElementById("story-list");
    const storyItems = storyList.querySelectorAll("li[data-username]");
    let deleted = false;

    storyItems.forEach(item => {
      if (item.getAttribute("data-username") === username) {
        storyList.removeChild(item);
        deleted = true;
      }
    });

    // Optionally hide list if empty
    if (!storyList.hasChildNodes()) {
      storyList.style.display = "none";
    }

    document.getElementById("username").value = "";
    showMessage(deleted ? "Story deleted successfully." : "No story found for that username.");
  })
  .catch(error => {
    console.error("Error deleting story:", error);
    showMessage("Failed to delete story. Please try again. " + error.message);
  });
});

document.getElementById("cgi-form").addEventListener("submit", function(event) {
  event.preventDefault();

  const name = document.getElementById("name").value.trim();
  const happy = document.getElementById("happy").checked ? "yes" : "no";
  const sad = document.getElementById("sad").checked ? "yes" : "no";

  const params = new URLSearchParams({
    name: name,
    happy: happy,
    sad: sad
  });

  fetch("/../cgi-bin/hello_process.py?" + params.toString(), {
    method: "GET",
  })
  .then(response => {
    if (!response.ok) {
      throw new Error("Network response was not ok");
    }
    return response.text();
  })
  .then(html => {
    // Open a new window
    const newWindow = window.open("", "_blank");

    if (newWindow) {
      newWindow.document.open();
      newWindow.document.write(html);  // Write CGI output
      newWindow.document.close();
      document.getElementById("cgi-form").reset();
    } else {
      console.error("Popup blocked. Please allow popups for this site.");
    }
  })
  .catch(error => {
    console.error("Fetch error:", error);
  });

});


