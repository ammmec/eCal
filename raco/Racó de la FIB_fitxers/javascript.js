$(document).ready(function() {	
	$('.noscript').hide();
	// Cantonades					
	/*$('.caixa').corner("round 8px");
	$('.dintre').corner("round 7px");
	$('.pestanyes li.normal').corner("top 5px");
	$('.pestanyes li.sel').corner("top 5px");
	$('.pestanyes ul.subnav').corner("top 5px");
	$('.pestanyes li.normal').nifty("small transparent top");
	$('.pestanyes li.sel').nifty("small transparent top");
	
	$('.pestanya_usuari').corner("top 5px");
	$('.pestanya_usuari .sortir').corner("5px");*/

	// Cantonades					
	/*$('.caixa').corners("6px transparent");
	$('.dintre').corners("6px transparent");*/
	/*if (!$.browser.msie || ($.browser.msie && $.browser.version.substr(0,1) > 7))
	{
		$('.pestanyes li.normal').corners("2px top transparent");
		$('.pestanyes li.sel').corners("2px top transparent");
		$('.pestanyes ul.subnav').corners("3px bottom transparent");
		$('.pestanya_usuari').corners("4px top transparent");
		$('.pestanya_usuari .sortir').corners("3px transparent");
	}*/
	/*$('.button').corners("4px transparent");
	$('.alert').corners("4px transparent");

	$('.form').corners("4px transparent");*/
	
	// El boto dret fa el mateix que el click
	/*
	if (!$.browser.msie || ($.browser.msie && $.browser.version.substr(0,1) > 7))
	{
		$("a.rss").bind("contextmenu",mostra_url);
	}
	*/

	// Ombra del logo
	//$("span#titol").dropShadow({left: 2, top: 2, opacity: 0.5, blur: 2});
	
	// Links amb KEY pels RSS (portada)
	/*if (!$.browser.msie || ($.browser.msie && $.browser.version.substr(0,1) > 7))
	{
		$(".rss").toggle(mostra_url,amaga_url);
	}*/
				
	// Cantonades del contingut
	// TODO: Desactivat perque el formulari no va
	// quan esta actiu
	//$('.contingut').corner("round 8px");			
	//$('.dintre_contingut').corner("round 7px");	

	// Taules estil zebra (quan n'hi hagi)
   	/*$('.contingut table:not(#reserves,.horari_petit_taula,#llegenda) tbody tr').each(function(){
		$(this).find('td:first-child').corners("3px left transparent");
		$(this).find('td:last-child').corners("3px right transparent");
	});
	$('.contingut table:not(.horari_petit_taula) thead').each(function(){
		$(this).find('th:first-child').corners("3px top-left transparent");
		$(this).find('th:last-child').corners("3px top-right transparent");
		$(this).find('td:first-child').corners("3px top-left transparent");
		$(this).find('td:last-child').corners("3px top-right transparent");
	});
	$('.contingut table:not(.horari_petit_taula) tfoot').each(function(){
		$(this).find('td:first-child').corners("3px bottom-left transparent");
		$(this).find('td:last-child').corners("3px bottom-right transparent");
	});*/
	/*$('.contingut table:not(#reserves,.horari_petit_taula,#llegenda,#resum_ects) tr').hover(
		function()
		{
			$(this).addClass('hover');
		},
		function(){
			$(this).removeClass('hover');
		}
	);*/
	
	$('.filtre').corners("3px bottom transparent");

	$('.fitxa').corners("3px transparent");
	$('.fitxa .field:last').css('border-bottom','none');
	$('.fitxa dl:last').css('border-bottom', 'none');
	
	$('.submenu ul').corners("3px transparent");
	$('.submenu ul li').corners("3px transparent");

	// I finalment... activar menus desplegables
	mainmenu();


	$('button:not(.no-jquery-ui)').button();
	$('button.add').button({
		icons: {
			primary: 'ui-icon-circle-plus'
		}
	});
	$('button.edit').button({
		icons: {
			primary: 'ui-icon-pencil'
		}
	});
	$('button.delete').button({
		icons: {
			primary: 'ui-icon-circle-close'
		}
	});
	$('button.download').button({
		icons: {
			primary: 'ui-icon-arrowthickstop-1-s'
		}
	});
	$('button.view').button({
		icons: {
			primary: 'ui-icon-circle-triangle-e'
		}
	});
	$('button.next').button({
		icons: {
			secondary: 'ui-icon-circle-arrow-e'
		}
	});
	$('button.next.no-text').button({
		icons: {
			secondary: 'ui-icon-circle-arrow-e'
		},
		text: false
	});
	$('button.prev').button({
		icons: {
			primary: 'ui-icon-circle-arrow-w'
		}
	});
	$('button.prev.no-text').button({
		icons: {
			primary: 'ui-icon-circle-arrow-w'
		},
		text: false
	});
	$('button.image').button({
		icons: {
			primary: 'ui-icon-image'
		}
	});
	$('button.search').button({
		icons: {
			primary: 'ui-icon-search'
		}
	});
	$('button.search.no-text').button({
		icons: {
			primary: 'ui-icon-search'
		},
		text: false
	});
	$('button.folder').button({
		icons: {
			primary: 'ui-icon-folder-collapsed'
		}
	});
	$( 'button.send' ).button({
		  icons: {
			  primary: "ui-icon-mail-closed" 
		  }
	});
	$( 'button.accept' ).button({
		  icons: {
			  primary: "ui-icon-check" 
		  }
	});
	$( 'button.reject' ).button({
		  icons: {
			  primary: "ui-icon-closethick" 
		  }
	});
	$('a.button').button();
	$('.botons_form input[type="submit"]').button();
	
	//$('.alert').effect('highlight', {color: '#f8a607'}, 4000);

	var lletresGrans = false;

	$('ul#eines li#tamany').click(function() {
        var ico = $('ul#eines li#tamany img');
		var url = ico.prop('src');
		if(!lletresGrans) {
			$('body').css('font-size', '110%');
			url = url.replace("augmentat","disminuir");
			ico.prop('src',url);
			$('ul#eines li#tamany a').attr('title', 'Disminuir lletres');
			lletresGrans = true;
			return false;
		} else  {
			$('body').css('font-size', '100%');
            url = url.replace("disminuir","augmentat");
            ico.prop('src',url);
			$('ul#eines li#tamany a').attr('title', 'Augmentar lletres');
			lletresGrans = false;
			return false;
		}
	});

	//$('ul#eines li#tamany').toggle(function(){
	//	$('body').css('font-size', '110%');
	//	//$('ul#eines li#tamany a').removeClass('augmentat').addClass('disminuit');
	//	$('ul#eines li#tamany img').attr({
	//		'src' : '/racoEstatic/img/icons/disminuir_lletres.gif',
	//		'alt' : 'Disminuir lletres'
	//	});
	//	$('ul#eines li#tamany a').attr('title', 'Disminuir lletres');
	//	return false;
	//}, function(){
	//	$('body').css('font-size', '100%');
	//	//$('ul#eines li#tamany a').removeClass('disminuit').addClass('augmentat');
	//	$('ul#eines li#tamany img').attr({
	//		'src' : '/racoEstatic/img/icons/augmentat_lletres.gif',
	//		'alt' : 'Augmentar lletres'
	//	});
	//	$('ul#eines li#tamany a').attr('title', 'Augmentar lletres');
	//	return false;
	//});
	
	var availableHours = ["08:00", "08:15", "08:30", "08:45", "09:00", "09:15", "09:30", "09:45", "10:00", "10:15",
			"10:30", "10:15", "10:30", "10:45", "11:00", "11:15", "11:30", "11:45", "12:00", "12:15", "12:30",
			"12:45", "13:00", "13:15", "13:30", "13:45", "14:00", "14:15", "14:30", "14:45", "15:00", "15:15",
			"15:30", "15:45", "16:00", "16:15", "16:30", "16:45", "17:00", "17:15", "17:30", "17:45", "18:00",
			"18:15", "18:30", "18:45", "19:00", "19:15", "19:30", "19:45", "20:00", "20:15", "20:30", "20:45", "21:00"];
	
	$('.timestamp').autocomplete({
		source: availableHours
	});
});