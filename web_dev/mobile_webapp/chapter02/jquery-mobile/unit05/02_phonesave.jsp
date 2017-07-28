<%@ page contentType="text/html;charset=euc-kr"%>
<%@ page import="java.util.*" %>

<%
request.setCharacterEncoding("utf-8");
String[] phone = {
	"",
	request.getParameter("pname"),
	request.getParameter("pimage"),
	request.getParameter("pcompany"),
	request.getParameter("pprice"),
	request.getParameter("pcolor")
};

List<String[]> phoneDb = (List<String[]>) session.getAttribute("phoneDb");
String pno = null;
for(int i=1; i<=20; i++) {
	pno = "p" + i;
	boolean exist = false;
	for(String[] p : phoneDb) {
		if(p[0].equals(pno)) {
			exist = true;
			break;
		}
	}
	if(exist == false) { break; }
}
phone[0] = pno;

phoneDb.add(phone);

response.sendRedirect("02_phonedetail.jsp?pno=" + pno);
%>