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
	e.target.style.border = '3px solid #FFFF00';
}

document.onmouseout = e => {
	e.target.style.border = '';
}

document.onclick = e => {
	e.target.remove();
	clear();
	document.body.style.overflow = 'auto';
	return false;
}
