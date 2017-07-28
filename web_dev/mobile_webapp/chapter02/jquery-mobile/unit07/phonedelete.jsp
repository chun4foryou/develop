<%@ page contentType="text/html;charset=euc-kr"%>
<%@ page import="java.util.*" %>

<%
String pno = request.getParameter("pno");
List<String[]> phoneDb = (List<String[]>) session.getAttribute("phoneDb");
String[] phone = null;
for(String[] p : phoneDb) {
	if(p[0].equals(pno)) {
		phoneDb.remove(p);
		break;
	}
}
%>

<jsp:forward page="phonelist.jsp?currentPage=1"/>