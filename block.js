function sel(elem) {
	if (!elem) {
		return '';
	}

	if (elem.id) {
		return '#' + elem.id;
	} else {
		var index = 1;
		var el = elem.previousElementSibling;
		while (el) {
			index++;
			el = el.previousElementSibling;
		}
		return sel(elem.parentElement) + ' > ' + elem.tagName.toLowerCase() + ':nth-child(' + index + ')';
	}
}

var last;

function clear() {
	document.onmouseover = null;
	document.onmouseout = null;
	document.onclick = null;
	last.style.border = '';
}

document.onkeyup = e => {
	if (e.key == 'Escape') {
		clear();
	}
}

document.onmouseover = e => {
	last = e.target;
	e.target.style.border = '3px solid #FF0000';
}

document.onmouseout = e => {
	e.target.style.border = '';
}

document.onclick = e => {
	console.log('blockit ##' + sel(e.target));
	e.target.remove();
	clear();
	document.body.style.overflow = 'auto';
	return false;
}
