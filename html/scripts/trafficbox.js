(function(doc, $, $m){
    
    function toggle(event, cancel) {
        var light = $(this),
        index = light.attr('name'),
        mode = light.val(),
        command = '/' + index + mode;
        if (!cancel) {
            $m.showPageLoadingMsg();
            $.post(command).fail(function(){
                mode = mode === '1' ? '0' : '1';
                light.val(mode).trigger('change', true);
            }).always(function(){
                $m.hidePageLoadingMsg();
            });
        }
    }

    function sync() {
        var lights = $("#lights").on('change', ':jqmData(role=slider)', toggle);
        $m.showPageLoadingMsg();
        $.get('/status.txt').done(function(status){
            $.each(status.split(''), function(index, mode){
                lights.find('[name=' + index + ']:jqmData(role=slider)').val(mode);
            });
        }).always(function(){
            lights.addClass('fade in')
            $m.hidePageLoadingMsg();
        });
    }

    $(sync);
    
})(document, jQuery, jQuery.mobile);