var intervalId=null;

var canvas; // global throttle canvas object
		
// resize the canvas to fill browser window dynamically
window.addEventListener('resize', resizeThrottleCanvas, false);

function resizeThrottleCanvas() {
	// adjust the width to the page width
	var canvasElement=document.getElementById("throttleCanvas");
	if ( canvasElement ) {
		canvasElement.width = window.innerWidth - 60; // needs to be slightly narrower than the page width
	}
	refreshThrottleVisualization();
}

function showTab(pageId) {
	// 1. show the correct div and hide the others
	var tabs = document.getElementById('tabs');
	for (var i = 0; i < tabs.childNodes.length; i++) {
		var node = tabs.childNodes[i];
		if (node.nodeType == 1) { /* Element */
			node.style.display = (node.id == pageId) ? 'block' : 'none';
		}
	}

	// 2. change the class of the selected tab
	var tabHeader = document.getElementById('tabHeader');
	var linkToActivate = document.getElementById(pageId + 'link');
	for (var i = 0; i < tabHeader.childNodes.length; i++) {
		var node = tabHeader.childNodes[i];
		if (node.nodeType == 1) { /* Element */
			node.className = (node == linkToActivate) ? 'on' : '';
		}
	}

	// 3. on the status && dashboard pages, set a 5s repeating interval to load the data
	if (pageId == 'status' || pageId == 'dashboard') {
		if ( intervalId ) {
			clearInterval(intervalId);
		}
		intervalId = setInterval(function(){loadData(pageId)}, 5000);
	} else {
		if (intervalId) {
			clearInterval(intervalId);
			intervalId = null;
		}
		
	}
	loadData(pageId);
}

// lazy load of page, replaces content of div with id==<pageId> with
// content of remote file <pageId>.htm
function loadPage(pageId) {
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.onreadystatechange = function() {
		if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
			document.getElementById(pageId).innerHTML = xmlhttp.responseText;
			if (pageId == 'dashboard') {
				generateGauges();
			}
		}
	};
	xmlhttp.open("GET", pageId + ".htm", true);
	xmlhttp.send();
}

// load data from dynamic xml and replace values in div's
function loadData(pageId) {
	try {
		var xmlhttp = new XMLHttpRequest();
		xmlhttp.onreadystatechange = function() {
			if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
				var root = xmlhttp.responseXML.firstChild;
				for (var i = 0; i < root.childNodes.length; i++) {
					var node = root.childNodes[i]; // scan through the nodes
					if (node.nodeType == 1 && node.childNodes[0]) {
						var name = node.nodeName;
						var value = node.childNodes[0].nodeValue;
						refreshGaugeValue(name, value);
						//if (name.indexOf('bitfield') == -1) { // a normal div/span to update <*>
							var target = document.getElementById(name);
							if (!target) { // id not found, try to find by name
								var namedElements = document.getElementsByName(name);
								if (namedElements && namedElements.length)
									target = namedElements[0];
							}
							if (target) { // found an element, update according to its type
								if (target.nodeName.toUpperCase() == 'DIV' || target.nodeName.toUpperCase() == 'SPAN')
									target.innerHTML = value;
							/*	if (target.nodeName.toUpperCase() == 'INPUT') {
									target.value = value;
									var slider = document.getElementById(name + "Level");
									if (slider)
										slider.value = value;
								} */
							}
						//} 
					}
				}				
			}
		};
		xmlhttp.open("GET", pageId + ".xml", true);
		xmlhttp.send();
	} catch (err) {
		//
	}
}

/*function getIntValue(id) {
	var node = document.getElementById(id);
	if (node)
		return parseInt(node.value);
	return 0;
} */

