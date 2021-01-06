(function($){
    $.widget("ui.throbber", {
        // default options
        options: {
            overlay: true,
            hidden: true,
            speed: 50,
            showlabel: false,
        },
        _setOption: function( key, value ) {
            $.Widget.prototype._setOption.apply( this, arguments );
            switch ( key ) {
            case "label":
                this.lb.html(value);
                break;
            case "showlabel":
                this.options.showlabel = value;
                break;
            default:
                break;
            }
        },

        _create: function() {
            var self = this;
            self.wasHidden = self.element.is(':hidden');
            self.element
            .appendTo( 'body' ) // force to be in body
            .addClass('ui-throbber');

            self.throbber = $('<div/>', 
                { 
                    'class': 'ui-throbber-image'
                }
                )
                .appendTo( self.element );

            self.overlay = $('<div/>', 
                { 
                    'class': 'ui-throbber-overlay'
                }
                )
                .hide()
            .appendTo( 'body' );

            self.lb = $('<div/>', 
                {
                    'html': "", 
                    'class': "ui-throbber-label ui-corner-all"
                }
                )
                .hide()
            .appendTo(' body' );


            if (self.options.hidden) {
                self.element.hide(); 
                self.overlay.hide(); 
            } else {
                self.element.fadeIn('fast');
                if(self.options.showlabel) {
                    self.lb.fadeIn('fast');
                }
                if( self.options.overlay ) {
                    self.overlay.show();
                }
            }

        },
        show: function() {
            var self = this;
            if( ! self.throbberId ) {
                self.throbber.css('left', 0);
                $(window).bind( 'resize.throbber', function() {
                    if( self.options.overlay ) {
                        self.overlay
                        .width($(window).width())
                        .height($(window).height());
                    }

                    self.element
                    .position(
                        {
                            'my': 'center',
                            'at': 'center',
                            'of': 'body',
                            'collision': 'none'
                        }
                    );
                    self.lb
                    .position(
                        {
                            'my': 'center',
                            'at': 'center',
                            'of': 'body',
                            'collision': 'none',
                            'offset': "0 32"
                        }
                    );
                }
                ).triggerHandler('resize.throbber' );

                if( self.options.overlay ) {
                    self.overlay.show();
                }
                self.element.fadeIn('fast');
                if(self.options.showlabel) {
                  self.lb.fadeIn('fast');
                }
                var throbberWidth = self.throbber.width();
                var throbberContainerWidth = self.element.width();
                self.throbberId = setInterval(function(){
                    var cur_left = Math.abs(self.throbber.position().left);

                    self.throbber.css(
                        {
                            'left': -((cur_left + throbberContainerWidth) % throbberWidth)
                        }
                    );


                }, self.options.speed);
            }

        },
        hide: function() {
            var self = this;
            if( self.throbberId ) {
                clearInterval( self.throbberId );
                $(window).unbind( 'resize.throbber' );
                self.throbberId = null;
                self.element.fadeOut('fast');
                self.lb.fadeOut('fast');
                if( self.options.overlay ) {
                    self.overlay.hide();
                }
            }	   
        },
        destroy: function() {
            $.Widget.prototype.destroy.apply(this, arguments); // default destroy
            this.throbber.remove();
            this.lb.remove();
            this.overlay.remove();
            if( this.wasHidden ) {
                this.element.hide();
            } else {
                this.element.show();
            }
        }
    }
);
}
)(jQuery);
