<%@ page contentType="text/html;charset=euc-kr"%>

<!DOCTYPE html> 
<html> 
	<head>
		<title>jQuery Mobile</title>
		<meta charset="euc-kr" /> 	
		<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0, user-scalable=no"/>
		
		<link rel="shortcut icon" href="../image/icon.png">
		<link rel="apple-touch-icon" href="../image/icon.png"> 
		
		<link href="../framework/jquery.mobile-1.0.css" rel="stylesheet" type="text/css" />
		<script src="../framework/jquery-1.6.4.js"></script>
		<script src="../framework/jquery.mobile-1.0.js"></script>
	</head> 

	<body>
		<div data-role="page">
			<div data-role="header" data-position="fixed">
				<h1>header</h1>
			</div>

			<div data-role="content">
				<%for(int i=1; i<=100; i++) {%>
						content <%=i%> <br/>
				<%}%>
			</div>

			<div data-role="footer" data-position="fixed">
				<h1>footer content</h1>
			</div>
		</div>
	</body>
</html>

