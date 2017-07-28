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
String[] dirtyPhone = {
	toKr(request.getParameter("pno")),
	toKr(request.getParameter("pname")),
	toKr(request.getParameter("pimage")),
	toKr(request.getParameter("pcompany")),
	toKr(request.getParameter("pprice")),
	toKr(request.getParameter("pcolor"))
};

String pno = request.getParameter("pno");
List<String[]> phoneDb = (List<String[]>) session.getAttribute("phoneDb");
String[] phone = null;
for(int i=0; i<phoneDb.size(); i++) {
	phone = phoneDb.get(i);
	if(phone[0].equals(dirtyPhone[0])) {
		phoneDb.remove(i);
		phoneDb.add(i, dirtyPhone);
		phone = dirtyPhone;
		break;
	}
}
%>

<jsp:forward page="phonedetail.jsp">
	<jsp:param name="pno" value="<%=phone[0]%>"/>
</jsp:forward>