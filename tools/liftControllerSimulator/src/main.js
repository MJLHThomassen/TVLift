const termkit = require("terminal-kit");
const term = termkit.terminal;

// Set up UI
term.fullscreen();
term.hideCursor();

var document = term.createDocument({
	palette: new termkit.Palette()
});

var layout = new termkit.Layout({
	parent: document,
	label: "label!",
	boxChars: "lightRounded",
	layout: {
		id: "main",
		widthPercent: 100,
		heightPercent: 100,
		rows: [
			{
				id: "row0",
				height: 3,
				columns: [
					{ id: "title" }
				]
			},
			{
				id: "row1",
				columns: [
					{ id: "input", widthPercent: 30 },
					{ id: "console" },
				]
			},
		]
	}
});

new termkit.Text({
	parent: document.elements.title,
	content: "Lift Controller Simulator",
	attr: {}
});

var consoleBox = new termkit.TextBox({
	parent: document.elements.console,
	scrollable: true,
	vScrollBar: true,
	lineWrap: true,
	autoWidth: true,
	autoHeight: true
});

var serverStatusToggleButton = new termkit.ToggleButton({
	parent: document.elements.input,
	y: 0,
	content: "Enable Server",
	turnedOnLeftPadding: "[x] ",
	turnedOffLeftPadding: "[ ] ",
	value: false
});

var liftStatusToggleButton = new termkit.ToggleButton({
	parent: document.elements.input,
	y: 1,
	content: "Enable Lift",
	turnedOnLeftPadding: "[x] ",
	turnedOffLeftPadding: "[ ] ",
	value: false
});

// Redirect console
console.log = (function(oldLog)
{
	return function (...args)
	{
		if(!consoleBox)
		{
			return;
		}
		
		for(const arg of args)
		{
			if(typeof arg === "object")
			{
				consoleBox.appendLog(JSON.stringify(arg));
			}
			else
			{
				consoleBox.appendLog(arg);
			}
		}
	};
})(console.log);

console.error = (function(oldError)
{
	return function (...args)
	{
		if(!consoleBox)
		{
			return;
		}

		for(const arg of args)
		{
			if(typeof arg === "object")
			{
				consoleBox.appendLog(JSON.stringify(arg));
			}
			else
			{
				consoleBox.appendLog(arg);
			}
		}
	};
})(console.error);

// Load Webserver
const Webserver = require("./webserver/webserver");
const webserver = new Webserver();
const liftService = require("./services/liftService");

// Set up interactions
serverStatusToggleButton.on("toggle", (value) =>
{
	if(value)
	{
		// Enable Server
		console.log("Enabling Server");
		webserver.start();
	}
	else
	{
		// Disable Server
		console.log("Disabling Server");
		webserver.stop();
	}
});

liftStatusToggleButton.on("toggle", (value) =>
{
	if(value)
	{
		// Enable Lift
		console.log("Enabling lift");
		liftService.status = "online";
	}
	else
	{
		// Disable Lift
		console.log("Disabling lift");
		liftService.status = "offline";
	}
});

let hasInputFocus = true;
term.on("key", function (key)
{
	switch (key)
	{
		case "RIGHT":
			hasInputFocus = false;
			document.giveFocusTo(consoleBox);
			break;
		case "LEFT":
			document.giveFocusTo(document.elements.input.children[0]);
			hasInputFocus = true;
			break;
		case "UP":
			if(hasInputFocus)
			{
				document.elements.input.focusPreviousChild();
			}
			break;
		case "DOWN":
			if(hasInputFocus)
			{
				document.elements.input.focusNextChild();
			}
			break;
		case "CTRL_C":
			term.grabInput(false);
			term.hideCursor(false);
			term.styleReset();
			term.clear();
			term.processExit(0);
			break;
	}
});

document.giveFocusTo(serverStatusToggleButton);