<%@ page contentType="text/html;charset=euc-kr"%>
<%@ page import="java.util.*" %>

<%
List<String[]> phoneDb = (List<String[]>) session.getAttribute("phoneDb");
if(phoneDb == null) {
	phoneDb = new ArrayList<String[]>();
	phoneDb.add(new String[] {
		"p1", "°¶·°½Ã A", "phone01.png", "»ï¼º", "600000", "°ËÁ¤"
	});
	phoneDb.add(new String[] {
		"p2", "¿ÉÆ¼¸Ó½º ¿ø", "phone02.png", "¿¤Áö", "500000", "Èò»ö"
	});
	phoneDb.add(new String[] {
		"p3", "°¶·°½Ã S", "phone03.png", "»ï¼º", "700000", "°ËÁ¤"
	});
	phoneDb.add(new String[] {
		"p4", "°¶·°½Ã S2", "phone04.png", "»ï¼º", "800000", "Èò»ö"
	});
	session.setAttribute("phoneDb", phoneDb);
}
%>

<div id="phonelist" data-role="page">
	<div data-role="header">
		<h1>Æù ¸ñ·Ï</h1>
	</div>  
	
	<div data-role="content">
		<ul data-role="listview">
			<%for(String[] phone : phoneDb) {%>
					<li>
						<a href="01_phonedetail.jsp?pno=<%=phone[0]%>">
							<table>
								<tr>
									<td style="padding-right: 5px">
										<img src="../image/<%=phone[2]%>" 
												style="width:30px; height:50px"/>
									</td>
									<td style="padding-left: 5px"><%=phone[1]%></td>
								</tr>
							</table>
						</a>
					</li>
			<%}%>
		</ul>
    </div>

	<div data-role="footer" data-position="fixed">
    	<div data-role="navbar">
    		<ul>
				<li><a href="#home" data-icon="home" 
						data-transition="slide" data-direction="reverse">¾ÛÈ¨</a></li>
			</ul>
		</div>
    </div>
</div>