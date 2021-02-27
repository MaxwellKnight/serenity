/*
 * Copyright (c) 2020-2021, Andreas Kling <kling@serenityos.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WebContentClient.h"
#include "OutOfProcessWebView.h"
#include <AK/Debug.h>

namespace Web {

WebContentClient::WebContentClient(OutOfProcessWebView& view)
    : IPC::ServerConnection<WebContentClientEndpoint, WebContentServerEndpoint>(*this, "/tmp/portal/webcontent")
    , m_view(view)
{
    handshake();
}

void WebContentClient::die()
{
    VERIFY(on_web_content_process_crash);
    on_web_content_process_crash();
}

void WebContentClient::handshake()
{
    send_sync<Messages::WebContentServer::Greet>();
}

void WebContentClient::handle(const Messages::WebContentClient::DidPaint& message)
{
    m_view.notify_server_did_paint({}, message.bitmap_id());
}

void WebContentClient::handle([[maybe_unused]] const Messages::WebContentClient::DidFinishLoading& message)
{
    m_view.notify_server_did_finish_loading({}, message.url());
}

void WebContentClient::handle(const Messages::WebContentClient::DidInvalidateContentRect& message)
{
    dbgln_if(SPAM_DEBUG, "handle: WebContentClient::DidInvalidateContentRect! content_rect={}", message.content_rect());

    // FIXME: Figure out a way to coalesce these messages to reduce unnecessary painting
    m_view.notify_server_did_invalidate_content_rect({}, message.content_rect());
}

void WebContentClient::handle(const Messages::WebContentClient::DidChangeSelection&)
{
#if SPAM_DEBUG
    dbgln("handle: WebContentClient::DidChangeSelection!");
#endif
    m_view.notify_server_did_change_selection({});
}

void WebContentClient::handle(const Messages::WebContentClient::DidRequestCursorChange& message)
{
    if (message.cursor_type() < 0 || message.cursor_type() >= (i32)Gfx::StandardCursor::__Count) {
        dbgln("DidRequestCursorChange: Bad cursor type");
        return;
    }
    m_view.notify_server_did_request_cursor_change({}, (Gfx::StandardCursor)message.cursor_type());
}

void WebContentClient::handle(const Messages::WebContentClient::DidLayout& message)
{
    dbgln_if(SPAM_DEBUG, "handle: WebContentClient::DidLayout! content_size={}", message.content_size());
    m_view.notify_server_did_layout({}, message.content_size());
}

void WebContentClient::handle(const Messages::WebContentClient::DidChangeTitle& message)
{
    dbgln_if(SPAM_DEBUG, "handle: WebContentClient::DidChangeTitle! title={}", message.title());
    m_view.notify_server_did_change_title({}, message.title());
}

void WebContentClient::handle(const Messages::WebContentClient::DidRequestScrollIntoView& message)
{
    dbgln_if(SPAM_DEBUG, "handle: WebContentClient::DidRequestScrollIntoView! rect={}", message.rect());
    m_view.notify_server_did_request_scroll_into_view({}, message.rect());
}

void WebContentClient::handle(const Messages::WebContentClient::DidHoverLink& message)
{
    dbgln_if(SPAM_DEBUG, "handle: WebContentClient::DidHoverLink! url={}", message.url());
    m_view.notify_server_did_hover_link({}, message.url());
}

void WebContentClient::handle(const Messages::WebContentClient::DidUnhoverLink&)
{
#if SPAM_DEBUG
    dbgln("handle: WebContentClient::DidUnhoverLink!");
#endif
    m_view.notify_server_did_unhover_link({});
}

void WebContentClient::handle(const Messages::WebContentClient::DidClickLink& message)
{
    m_view.notify_server_did_click_link({}, message.url(), message.target(), message.modifiers());
}

void WebContentClient::handle(const Messages::WebContentClient::DidMiddleClickLink& message)
{
    m_view.notify_server_did_middle_click_link({}, message.url(), message.target(), message.modifiers());
}

void WebContentClient::handle(const Messages::WebContentClient::DidStartLoading& message)
{
    m_view.notify_server_did_start_loading({}, message.url());
}

void WebContentClient::handle(const Messages::WebContentClient::DidRequestContextMenu& message)
{
    m_view.notify_server_did_request_context_menu({}, message.content_position());
}

void WebContentClient::handle(const Messages::WebContentClient::DidRequestLinkContextMenu& message)
{
    m_view.notify_server_did_request_link_context_menu({}, message.content_position(), message.url(), message.target(), message.modifiers());
}

void WebContentClient::handle(const Messages::WebContentClient::DidGetSource& message)
{
    m_view.notify_server_did_get_source(message.url(), message.source());
}

void WebContentClient::handle(const Messages::WebContentClient::DidJSConsoleOutput& message)
{
    m_view.notify_server_did_js_console_output(message.method(), message.line());
}

OwnPtr<Messages::WebContentClient::DidRequestAlertResponse> WebContentClient::handle(const Messages::WebContentClient::DidRequestAlert& message)
{
    m_view.notify_server_did_request_alert({}, message.message());
    return make<Messages::WebContentClient::DidRequestAlertResponse>();
}

OwnPtr<Messages::WebContentClient::DidRequestConfirmResponse> WebContentClient::handle(const Messages::WebContentClient::DidRequestConfirm& message)
{
    auto result = m_view.notify_server_did_request_confirm({}, message.message());
    return make<Messages::WebContentClient::DidRequestConfirmResponse>(result);
}

OwnPtr<Messages::WebContentClient::DidRequestPromptResponse> WebContentClient::handle(const Messages::WebContentClient::DidRequestPrompt& message)
{
    auto result = m_view.notify_server_did_request_prompt({}, message.message(), message.default_());
    return make<Messages::WebContentClient::DidRequestPromptResponse>(result);
}

}
