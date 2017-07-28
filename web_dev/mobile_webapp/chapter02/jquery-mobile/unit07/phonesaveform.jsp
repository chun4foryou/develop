<%@ page contentType="text/html;charset=euc-kr" %>

<div id="phonesaveform" data-role="page" data-theme="a">
	<script type="text/javascript">
		function phonesave() {
			var pname = $("#pname").attr("value");
			var pcolor = $("#phonesaveform input[name=pcolor]:checked").attr("value");
			var pimage = $("#pimage").attr("value");
			var pcompany = $("#pcompany").attr("value");
			var pprice = $("#pprice").attr("value");

			var param = "pname="+encodeURI(pname);
			param += "&pimage="+pimage;
			param += "&pcolor="+encodeURI(pcolor);
			param += "&pcompany="+encodeURI(pcompany);
			param += "&pprice="+pprice;
			$.mobile.changePage("phonesave.jsp?" + param);
		}

		function selectphone(pimage) {
			$("#pimage").attr("value", pimage);
		}
	</script>

	<div data-role="header" data-position="fixed" data-theme="a">
		<h1>폰 등록</h1>
		<a href="#" data-rel="back" data-icon="arrow-l">이전</a>
		<a href="javascript:phonesave();" data-icon="arrow-r" data-iconpos="right">등록</a>
	</div>  
	
	<div data-role="content">
		<form id="phonesaveform" method="post" action="#">
			<div data-role="fieldcontain">
				<label for="pname">상품명:</label>
				<input id="pname" type="text" name="pname" value="갤럭시 New"/> 
			</div>

			<div data-role="fieldcontain">
				<label for="pimage">이미지:</label>
				<input id="pimage" type="text" name="text" value="phone01.png" readonly>
				<div data-role="collapsible" data-theme="b" data-content-theme="e">
					<h3>이미지 선택</h3>
					<table style="width:100%">
						<tr><td>
							<%for(int i=1; i<=20; i++) {
									String pimage = null;
									if(i<10) {
										pimage = "phone0" + i + ".png";
									} else {
										pimage = "phone" + i + ".png";
									}%>
									<a href="javascript:selectphone('<%=pimage%>');">
										<img src="../image/<%=pimage%>" 
												style="width:30px; height:50px"/>
									</a>
							<%}%>
						</td></tr>
					</table>
				</div>
			</div>

			<div data-role="fieldcontain">
				<fieldset data-role="controlgroup" data-type="horizontal">
					<legend>색상:</legend>
					<input id="radio1" type="radio" name="pcolor" value="검정" checked="checked"/>
					<label for="radio1">깜뎅이</label>
					<input id="radio2"  type="radio" name="pcolor" value="흰색"/>
					<label for="radio2">흰둥이</label>
				</fieldset>
			</div>

			<div data-role="fieldcontain">
				<label for="pcompany">제조사:</label>
				<input id="pcompany" type="text" name="pcompany" value="삼성"/> 
			</div>

			<div data-role="fieldcontain">
				<label for="pprice">상품가:</label>
				<input id="pprice" type="number" name="pprice" value="300000"/> 
			</div>
		</form>
	</div>

	<div data-role="footer" data-position="fixed" data-theme="a">
		<div data-role="navbar">
			<ul>
				<li><a href="#home" data-icon="home" 
							data-transition="slide" 
							data-direction="reverse">앱홈</a></li>
				<li><a href="phonelist.jsp?currentPage=1" 
							data-icon="grid"
							data-transition="slide" 
							data-direction="reverse">목록처음</a></li>
			</ul>
		</div>
	</div>
</div>
