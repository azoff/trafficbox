(function(doc, $, $m){
    
    var switches, monitor;
    
    function send(command, type) {
        type = type || 'POST';
        $m.showPageLoadingMsg();
        return $.ajax({ url: command, type: type }).always(show).then(sync);
    }
    
    function toggle(event, cancel) {
        var light = $(this),
        mode = light.val(),        
        index = light.attr('name'),
        command = '/' + index + mode;
        if (!cancel) { send(command); }
    }

    function sync(status) {
        monitor.status = status.split('');
        $.each(monitor.status, function(index, mode){
            var slider = switches.find('[name=' + index + ']:jqmData(role=slider)').val(mode);
            try { slider.slider('refresh'); } 
            catch (e) { slider.trigger('create'); }
        });
    }

    function show() {
        switches.addClass('fade in');
        $m.hidePageLoadingMsg();
    }
    
    function buildPending() {
        var opp = monitor.status[1] === '1' ? '0' : '1';
        if (monitor.pending) {
            send('/1' + opp).then(function(){
                setTimeout(buildPending, 1000);
            });
        }
    }
    
    function buildSuccess() {
        send('/21');
    }
    
    function buildFailure() {
        send('/01');
    }
    
    function buildReset() {
        $.each(monitor.status, function(index, mode){
            if (mode === '1') {
                send('/' + index + '0');
            }
        });
    }
        
    function poller() {
        var host = monitor.find('#host').val(),
        job = monitor.find('#job').val(),
        enable = monitor.find('#enable'),
        interval = parseInt(monitor.find('#interval').val(), 10),
        url = [host, 'job', job, 'api/json?jsonp=?'].join('/');
        if (enable.val() === "1") {
            $.getJSON(url).then(function(data){
                if (data.lastBuild.number > data.lastCompletedBuild.number && !monitor.pending) {
                    monitor.pending = true;
                    buildReset();
                    buildPending();
                } else if (data.lastBuild.number <= data.lastCompletedBuild.number && 
                    (monitor.pending === true || monitor.pending === undefined)) {
                    monitor.pending = false;
                    buildReset();
                    if (data.lastSuccessfulBuild && data.lastBuild.number === data.lastSuccessfulBuild.number) {
                        buildSuccess();
                    } else {
                        buildFailure();
                    }
                }
                setTimeout(poller, interval);              
            }).fail(function(){
                enable.val('0').slider('refresh');
            });
        }        
    }

    function start() {
        switches = $("#switches").on('change', ':jqmData(role=slider)', toggle);
        monitor = $('#monitor').on('change', ':jqmData(role=slider)', poller);
        send('/', 'OPTIONS');
    }
    
    $m.defaultPageTransition = 'flow';
    $(start);
    
})(document, jQuery, jQuery.mobile);
