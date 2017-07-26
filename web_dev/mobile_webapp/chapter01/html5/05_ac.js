function startPage() {
	if(window.applicationCache) {
		alert("어플리케이션 캐시를 지원");
	} else {
		alert("어플리케이션 캐시를 미지원");
	}
}
