/**
 * 
 */

//jQuery(document).load(function() {
window.onload = function(){
	//setInterval(function(){
		var inputs = document.getElementsByTagName("input");
		

		for (var i = 0; i < inputs.length; i++) {
			if (inputs[i].name.indexOf('modules[') === 0) {
				inputs[i].onclick = function() {
					var name = this.name.substr('modules['.length);
					var tokens = name.split(']');
					var module = tokens[0];
					var action = tokens[1].substr(1,tokens[1].length-1);
					
					if (action === 'enable') {
						if (this.checked) {
							var dl = document.getElementById('edit-modules-' + module + '-download');
							if (!dl.disabled) {
								dl.checked = true;
							}	
						}
					}
					else if (action === 'download') {
						if (!this.checked) {
							document.getElementById('edit-modules-' + module + '-enable').checked = false;
						}
					}
				};
			}
		}
//}, 3000);
};