<%@ page contentType="text/plain;charset=euc-kr"%>

<%
String uid = request.getParameter("uid");
String upassword = request.getParameter("upassword");
%>

<div id="login" data-role="page" data-theme="a">
	<div data-role="header" data-theme="a">
		<h1>로그인 결과</h1>
	</div>

	<div data-role="content">
		<%if(uid.equals("user") && upassword.equals("12345")) {%>
				<center>
					<h3>로그인 성공</h3>
					<a	href="#home" 
						data-icon="home"
						data-role="button"
						data-inline="true"
						data-direction="reverse">앱홈</a>
					<script type="text/javascript">
						$("#home").attr("data-login", "ok");
					</script>
				</center>
		<%} else {%>
				<center>
					<%if(!uid.equals("user")) {%>
							<h3>아이디기 존재하지 않음</h3>
					<%} else {%>
							<h3>패스워드가 틀림</h3>
					<%}%>
					<div class="ui-grid-a">
						<div class="ui-block-a">
							<a	href="loginform.html" 
								data-role="button"
								data-icon="gear"
								data-direction="reverse">로그인폼</a>
						</div>
						<div class="ui-block-b">
							<a	href="#home" 
								data-icon="home"
								data-role="button"
								data-direction="reverse">앱홈</a>
						</div>
					</div>
				</center>
		<%}%>
	</div>
</div>