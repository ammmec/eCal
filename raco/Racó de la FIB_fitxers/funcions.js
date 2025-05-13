function get_rss_feed(url, id, n, categoria)
{
	//if (categoria == null) alert("Sense categoria");
	//else alert("Categoria: " + categoria);
	$(id).html('<p class="loading">Carregant...</p>');
	$.ajax({
		url: url,
		dataType: 'xml',
		success: function(data)
		{
			$(id).html('');
			$(data).find('item').each(function(i)
			{
				if (i >= n && categoria == null) return;			
				var $item = $(this);
				
				var title = $item.find('>title').text();
				var link = $item.find('>link').text();
				var pubDate = $item.find('pubDate').text();
				var categories= $item.find('category').text();
				// Si tenim categoria, l'hem de crear i despres afegir-li
				if (categories == "") on=id;
				else
				{
					c = categories.split(", ");
					// El ID es c[0], que es numeric. La resta son titols.
					id_categoria="c"+c[0];
					if ($("#"+id_categoria).text() == "" && (categoria == null || (categoria != null && 'Fòrum ' + categoria == c[2])))
					{
						$(id).append('<h4 class="rss_title">' + c[1] + ' (' + c[2] + ')</h4><div id="' + id_categoria + '" class="rss_content"></div>');					    
					}
					on = "#" + id_categoria;
				}
				var a=$("<a href="+link+"/>").text(title)
                var li=$("<h5>")
                li.append(a)
                $(on).append(li);
			});
		},
		error: function(a, b, c)
		{
			$(id).html('<p>El contingut no est&agrave; disponible en aquest moment.</p>');
		}
	});
}

function get_latest_forum_topics(url, id, n)
{
	$(id).html('<p class="loading">Carregant...</p>');
	$.ajax({
		url: url,
		dataType: 'xml',
		success: function(data)
		{
			$(id).html('<ul style="margin: 0 !important; padding: 0 !important;"></ul>');
			$(data).find('item').each(function(i)
			{
				if (i >= n && categoria == null) return;
				
				var title = $(this).find('title').text();
				var link = $(this).find('link').text();
				var data = new Date(Date.parse($(this).find('pubDate').text()));
				var categories = $(this).find('category').text().split(", ");
				
				$(id + ' ul').append('<li class="lastest-item"><p><a href="' + link + '" title="' + title + 
									 '">' + title + '</a></p><p><abbr class="timeago" title="' + ISODateString(data) + 
									 '">' + prettyDate(data) + '</abbr> a <a href="https://raco.fib.upc.edu/forum/forums/show/' + 
									 categories[0].split('-')[0] + '.page" title="' + categories[1] + ' (' + 
									 categories[2] + ')">' + categories[1] + '</a></p></li>').find('.timeago').timeago();
			});
		},
		error: function(a, b, c)
		{
			$(id).html('<p>El contingut no est&agrave; disponible en aquest moment.</p>');
		}
	});
}
	
function mostra_url (event){
	$(".popup_rss").hide().remove();
	// Demanem via ajax la URL que hi ha al link si s'ha de generar
	// Si no, posem el valor a la caixa i llestos
	var link=this;
	var content = '<input type="text" value="';
	if (link.href.indexOf("out.Genera")!=-1) {
		$.get(				
			link.href,
			function(response){
				content += response;
				/*div.show();		 		
				div.find("input").val(response);
				div.find("input").select();	*/				
			});
	} else {
		/*div.show();		 		
		div.find("input").val(link.href);
		div.find("input").select();	*/
		content += link.href;
	}
	content += '" />';
	/*
	$(this).qtip({
		content: {
			text: content,
		},
		show: 'click',
		hide: 'unfocus',
		style: {
			border: {
				width: 5,
				radius: 3
			},
			background: '#333333',
			name: 'dark',
			width: 50 + 'em',
			opacity: 0.95,
			padding: 0.2 + 'em'
		}
	});
	*/
	return false;
}		
		
function amaga_url (e){
	$(".popup_rss").hide();
}		
		

function omple_mails(data) {
	$('#mails_load p').remove();
	$.each(data.mails, function(i, mail)
	{
		var subject = mail.subject;
		var from = mail.from;
		var estil = 'read';
		if (mail.unseen == 'U')
		{
			estil = 'unread';
			subject = '<b>' + mail.subject + '</b>';
			from = '<b>' + mail.from + '</b>';
		}
		var link_subject = '<a href="' + mail.url + '" title="' + mail.subject + '" />' + mail.subject + '</a>';
		var link_from = '<a href="' + mail.url  + '" title="' + mail.from  + '">' + from + '</a>';
		$('#mails').append('<tr class="' + estil + '"><td class="mail_from">' + link_from + '</td><td class="mail_subject">' + link_subject + '</td></tr>');
	});
	$('#numMails').text(data.numMails);
	$('#numNoLlegits').text(data.numNoLlegits);
	$('.infomails').show();
	$('#mails tr:odd').addClass("alt");
}

function mainmenu(){
	// Els menus no seleccionats i el d'assignatures (en qualsevol estat) es depleguen
	$("#nav li.normal, #nav li.assignatures.sel").hover(
		function(){
			h = $(this).outerHeight();		
			$(this).find("ul").css("top", h);
			$(this).addClass("hover");
	    },
	    function(){
	    	$(this).delay(2000).removeClass("hover");
	    });    
}


var urlWebServiceNotifications = "/servlet/raco.webservices.NotificationsList";
function load_notifications()
{
	$.ajax({
		url: '/servlet/raco.webservices.NotificationsList',
		beforeSend: function()
		{
			$('.notifications').html('');
		},
		success: function(data)
		{
			$('#pendingNotifications').hide();
			$.each(data, render_notification);
			
			setTimeout(function(){mark_notifications_as_read(data);}, 3000);
		},
		error: function(a, b, c)
		{
			alert(a + " " + b + " " + c);
		}
	});
}

function render_notification(i, item)
{
	var data = new Date(Date.parse(item.when));
	var unseen = item.unseen == "Y" ? ' unseen' : '';
	
	$('.notifications').append('<li id="notification-' + item.id + '" class="notification' + unseen + '"><img src="https://raco.fib.upc.edu/servlet/lcfib.pub.FotoPublica?p_username=' + item.who + '" alt="" class="foto" /><p class="message"><a href="' + item.url + '" title="">' + item.message + '</a> <span class="date"><abbr class="timeago" title="' + ISODateString(data) + '">' + prettyDate(data) + '</abbr></span></li>').find('.timeago').timeago();
}

function ISODateString(d) {
	function pad(n)
	{
		return n < 10 ? '0' + n : n;
	}
	
	return d.getUTCFullYear()+'-'
	+ pad(d.getUTCMonth()+1)+'-'
	+ pad(d.getUTCDate())+'T'
	+ pad(d.getUTCHours())+':'
	+ pad(d.getUTCMinutes())+':'
	+ pad(d.getUTCSeconds())+'Z'
}

function prettyDate(d)
{
	return d.toLocaleDateString() + ' ' + d.toLocaleTimeString();
}

function update_pending_notifications()
{
	$.ajax({
		url: '/servlet/raco.webservices.PendingNotifications',
		success: function(data)
		{
			if (data.pending != 0) $('#pendingNotifications').html(data.pending).show();
			else $('#pendingNotifications').hide();
		}
	});
}

function mark_notifications_as_read(data)
{
	var ids = new Array();
	$.each(data, function(i, item){
		ids.push(item.id);
	});
	
	$.ajax({
		url: '/servlet/raco.webservices.MarkNotificationsAsRead',
		type: 'POST',
		data:
		{
			notifications: ids
		},
		success: function(data)
		{
			$.each(ids, function(i, id){
				$('#notification-' + id).removeClass('unseen'); 
			});
		},
		error: function(a, b, c)
		{
			
		}
	});
}

function cercar(tipus,q) {
	var redirectURL;	
	if (tipus=="estudiants")
	{
		redirectURL = "/comunitat/cercar-estudiants.jsp?cerca=" + q;
		document.location=redirectURL;
	}
	else if (tipus=="personal")
	{
		redirectURL = "http://cercador.upc.edu/cercaUPC/search?q=" + q + "&site=ADR_TELFS&config=3&hl=ca&op=2";
		window.open(redirectURL,"_blank");
	}
	else //cerquem per defecte al web de la FIB
	{
		redirectURL = "http://www.fib.upc.edu/fib/cercar.html?q="+q;
		window.open(redirectURL,"_blank");
	}
	return false;
}
