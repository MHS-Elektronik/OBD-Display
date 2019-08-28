/**
 * Gauge initialization
 *
 */
function refreshGaugeValue(setting, value) {
	var id = setting + 'Gauge';
	var gauge = document.getElementById(id);
	if ( gauge ) {
		if ( !value ) {
			var div = document.getElementById(setting);
			if ( div ) {
				value = div.innerHTML;
			}
		}
		if ( value ) {
			Gauge.Collection.get(id).setValue(value);
		}
	}
}

function generateGauges() {

	var SpeedGauge = new Gauge({
		renderTo    : 'SpeedGauge',
		width       : 350,
		height      : 350,
		glow        : true,
		units       : 'km/h',
		title       : "Tacho",
		minValue    : 0,
		maxValue    : 240,
		majorTicks  : ['0','20','40','60','80','100','120','140','160','180', '200', '220', '240'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },

	
		highlights  : [		
			{ from : 0, to : 240, color : 'rgba(0, 255,  0, .75)' }
		],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	SpeedGauge.draw();

 	var RpmGauge = new Gauge({
		renderTo    : 'RpmGauge',
		width       : 350,
		height      : 350,
		glow        : true,
		units       : 'x100',
		title       : "RPM",
		minValue    : 0,
		maxValue    : 10000,
		majorTicks  : ['0','1','2','3','4','5','6','7','8','9','10'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 5, "dec" : 0 },
		highlights  : [		
			{ from : 0, to : 8000, color : 'rgba(0, 255,  0, .75)' },
			{ from : 8000, to : 10000, color : 'rgba(255, 0, 0, .75)' }
		],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});
	
	RpmGauge.draw();

	var EngineLoadGauge = new Gauge({
		renderTo    : 'EngineLoadGauge',
		width       : 220,
		height      : 220,
		glow        : true,
		units       : '%',
		title       : "TPS",
		minValue    : 0,
		maxValue    : 100,
		majorTicks  : ['0','20','40','60','80','100'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },
    
		highlights  : [		
			{ from : 0, to : 100, color : 'rgba(0, 255,  0, .75)' }
		],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	EngineLoadGauge.draw();
  
	var MAPGauge = new Gauge({
		renderTo    : 'MAPGauge',
		width       : 220,
		height      : 220,
		glow        : true,
		units       : 'kPa',
		title       : "MAP",
		minValue    : 0,
		maxValue    : 250,
		majorTicks  : ['0','50','100','150','200','250'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },

	
		highlights  : [
			{ from : 0,   to : 250, color : 'rgba(0, 255, 0, .75)' }      
			],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	MAPGauge.draw();

	var WaterTempGauge = new Gauge({
		renderTo    : 'WaterTempGauge',
		width       : 220,
		height      : 220,
		glow        : true,
		units       : 'C',
		title       : "Wasser",
		minValue    : 0,
		maxValue    : 120,
		majorTicks  : ['0','20','40','60','80','100','120'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },
		highlights  : [
			{ from : 0,   to : 100, color : 'rgba(0, 255,  0, .75)' },
			{ from : 100, to : 110, color : 'rgba(255, 255, 0, .75)' },
			{ from : 110, to : 120, color : 'rgba(255, 0, 0, .75)' }
		],

		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	WaterTempGauge.draw();
  
	var FuelLevelGauge = new Gauge({
		renderTo    : 'FuelLevelGauge',
		width       : 220,
		height      : 220,
		glow        : true,
		units       : '%',
		title       : "Tank",
		minValue    : 0,
		maxValue    : 100,
		majorTicks  : ['0','20','40','60','80','100'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },
    
		highlights  : [		
			{ from : 0, to : 100, color : 'rgba(0, 255,  0, .75)' }
		],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	FuelLevelGauge.draw();      
}