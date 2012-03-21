(function(doc, $, $m){
    
    var lights;
    
    function toggle(event, cancel) {
        var light = $(this),
        mode = light.val(),        
        index = light.attr('name'),
        command = '/' + index + mode;
        if (!cancel) {
            $m.showPageLoadingMsg();
            $.post(command).fail(start).always(show).then(sync);
        }
    }

    function sync(status) {
        $.each(status.split(''), function(index, mode){
            lights.find('[name=' + index + ']:jqmData(role=slider)').val(mode).slider('refresh');
        });
    }

    function show() {
        lights.addClass('fade in');
        $m.hidePageLoadingMsg();
    }

    function start() {
        lights = $("#lights").on('change', ':jqmData(role=slider)', toggle);
        $m.showPageLoadingMsg();
        $.ajax({ url: '/', type: 'OPTIONS' }).always(show).then(sync);
    }

    $(start);
    
})(document, jQuery, jQuery.mobile);