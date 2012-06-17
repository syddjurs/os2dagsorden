function init(name, meeting_id, bullet_point_id, bilag_id) {

    jQuery(document).ready(function() {
	var token = jQuery.get('http://192.168.42.179:5001/auth/'+name);
	
        jQuery('#content').annotator().annotator('addPlugin', 'Auth', {
	   // tokenUrl: 'http://192.168.42.179:5001/auth/'+name,
	    token: token,
	});
	
	jQuery('#content').annotator().annotator('addPlugin', 'Touch', {
	  force: location.search.indexOf('force') > -1,
	  useHighlighter: location.search.indexOf('highlighter') > -1
	});
	
	jQuery('#content').annotator().annotator('addPlugin', 'Filter');
	
	jQuery('#content').annotator().annotator('addPlugin', 'Store', {
	  // The endpoint of the store on your server.
	  prefix: 'http://192.168.42.179:5000/',
	  annotationData: {
             'drupal_user': name,
	     'bilag_id': bilag_id,
	     'bullet_point_id': bullet_point_id,
	     'meeting_id': meeting_id
	  },
	  loadFromSearch: {
	     'drupal_user': name,
	     'bilag_id': bilag_id,
	     'bullet_point_id': bullet_point_id,
	     'meeting_id': meeting_id
          }
	});
	

     });

}
