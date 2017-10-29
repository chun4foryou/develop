<%@ page contentType="text/html;charset=euc-kr"%>
<%@ page import="java.util.*" %>

<%
String pno = request.getParameter("pno");
List<String[]> phoneDb = (List<String[]>) session.getAttribute("phoneDb");
String[] phone = null;
for(String[] p : phoneDb) {
	if(p[0].equals(pno)) {
		phone = p;
		break;
	}
}
%>

<div id="phonedetail" data-role="page">
	<div data-role="header">
		<h1>폰 상세 내용</h1>
	</div>  
	<div data-role="content">
		<ul data-role="listview">
			<li>
				<table>
					<tr>
						<td style="padding-right: 5px">
							<img src="../image/<%=phone[2]%>"/>
						</td>
						<td style="padding-left: 5px">
							<table>
								<tr style="padding: 5px">
									<td style="padding: 5px">모델명:</td>
									<td><%=phone[1]%></td>
								</tr>
								<tr style="padding: 5px">
									<td style="padding: 5px">제조사:</td>
									<td><%=phone[3]%></td>
								</tr>
								<tr style="padding: 5px">
									<td style="padding: 5px">상품가:</td>
									<td><%=phone[4]%></td>
								</tr>
								<tr style="padding: 5px">
									<td style="padding: 5px">색상:</td>
									<td><%=phone[5]%></td>
								</tr>
							</table>
						</td>
					</tr>
				</table>
			</li>
		</ul>
    </div>
    
	<div data-role="footer" data-position="fixed">
    	<div data-role="navbar">
    		<ul>
				<li><a href="#home" data-icon="home" 
						data-transition="slide" data-direction="reverse">앱홈</a></li>
				<li><a href="03_phonelist.jsp" data-icon="grid"
						data-transition="slide" data-direction="reverse">목록</a></li>
			</ul>
		</div>
    </div>
</div>