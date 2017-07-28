<%@ page contentType="text/html;charset=euc-kr"%>
<%@ page import="java.util.*" %>

<%!
private String toKr(String value) {
	String result = null;
	try {
		result = new String(value.getBytes("8859_1"), "utf-8");
	} catch(Exception e) {}
	return result;
}
%>

<%
String[] phone = {
	"",
	toKr(request.getParameter("pname")),
	toKr(request.getParameter("pimage")),
	toKr(request.getParameter("pcompany")),
	toKr(request.getParameter("pprice")),
	toKr(request.getParameter("pcolor"))
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
%>

<jsp:forward page="phonedetail.jsp">
	<jsp:param name="pno" value="<%=phone[0]%>"/>
</jsp:forward>