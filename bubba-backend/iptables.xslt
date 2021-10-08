<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- Converts from simple xml iptables format to iptables-save format
     Copyright 2006 UfoMechanic
     Author: azez@ufomechanic.net
     This code is distributed and licensed under the terms of GNU GPL v2

     This sample usage outputs roughly want goes in
       iptables-save | iptables-xml -c | xsltproc iptables.xslt -
     -->
<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method = "text" />
  <xsl:strip-space elements="*" />

  <!-- output conditions of a rule but not an action -->
  <xsl:template match="iptables-rules/table/chain/rule/conditions/*">
    <!-- <match> is the psuedo module when a match module doesn't need to be loaded
         and when -m does not need to be inserted -->
    <xsl:if test="name() != 'match'">
      <xsl:text> -m </xsl:text><xsl:value-of select="name()"/>
    </xsl:if>
    <xsl:apply-templates select="node()"/>
  </xsl:template>

  <!-- delete the actions or conditions containers, and process child nodes -->
  <xsl:template match="iptables-rules/table/chain/rule/actions|table/chain/rule/conditions">
    <xsl:apply-templates select="*"/>
  </xsl:template>

  <xsl:template match="iptables-rules/table/chain/rule/actions/goto">
    <xsl:text> -g </xsl:text>
    <xsl:apply-templates select="*"/>
    <xsl:text>&#xA;</xsl:text>
  </xsl:template>
  <xsl:template match="iptables-rules/table/chain/rule/actions/call">
    <xsl:text> -j </xsl:text>
    <xsl:apply-templates select="*"/>
    <xsl:text>&#xA;</xsl:text>
  </xsl:template>
  <!-- all other actions are module actions -->
  <xsl:template match="iptables-rules/table/chain/rule/actions/*">
    <xsl:text> -j </xsl:text><xsl:value-of select="name()"/>
    <xsl:apply-templates select="*"/>
    <xsl:text>&#xA;</xsl:text>
  </xsl:template>

  <!-- all child action nodes -->
  <xsl:template match="iptables-rules/table/chain/rule/actions//*|iptables-rules/table/chain/rule/conditions//*" priority="0">
    <xsl:if test="@invert=1"><xsl:text> !</xsl:text></xsl:if>
    <xsl:text> -</xsl:text>
    <!-- if length of name is 1 character, then only do 1 - not 2 -->
    <xsl:if test="string-length(name())&gt;1">
      <xsl:text>-</xsl:text>
    </xsl:if>
    <xsl:value-of select="name()"/>
    <xsl:text> </xsl:text>
    <xsl:apply-templates select="node()"/>
  </xsl:template>

  <xsl:template match="iptables-rules/table/chain/rule/actions/call/*|iptables-rules/table/chain/rule/actions/goto/*">
    <xsl:value-of select="name()"/>
    <!-- I bet there are no child nodes, should we risk it? -->
    <xsl:apply-templates select="node()"/>
  </xsl:template>

  <!-- output the head of the rule, and any conditions -->
  <xsl:template name="rule-head">
    <xsl:if test="string-length(@packet-count)+string-length(@byte-count)">
      <xsl:call-template name="counters"><xsl:with-param name="node" select="."/></xsl:call-template>
      <xsl:text> </xsl:text>
    </xsl:if>
    <xsl:text>-A </xsl:text><!-- a rule must be under a chain -->
    <xsl:value-of select="../@name" />
    <xsl:apply-templates select="conditions"/>
  </xsl:template>

  <!-- Output a single rule, perhaps as multiple rules if we have more than one action -->
  <xsl:template match="iptables-rules/table/chain/rule">
    <xsl:choose>
      <xsl:when test="count(actions/*)&gt;0">
        <xsl:for-each select="actions/*">
          <!-- and a for-each to re-select the rule as the current node, to write the rule-head -->
          <xsl:for-each select="../..">
            <xsl:call-template name="rule-head"/>
          </xsl:for-each>
          <!-- now write the this action -->
          <xsl:apply-templates select="."/>
        </xsl:for-each>
      </xsl:when>
      <xsl:otherwise>
        <!-- no need to loop if there are no actions, just output conditions -->
        <xsl:call-template name="rule-head"/>
        <xsl:text>&#xA;</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="iptables-rules/table">
    <xsl:text># Generated by iptables.xslt&#xA;</xsl:text>
    <xsl:text>*</xsl:text><xsl:value-of select="@name"/><xsl:text>&#xA;</xsl:text>
    <!-- Loop through each chain and output the chain header -->
    <xsl:for-each select="chain">
      <xsl:text>:</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text> </xsl:text>
      <xsl:choose>
        <xsl:when test="not(string-length(@policy))"><xsl:text>-</xsl:text></xsl:when>
        <xsl:otherwise><xsl:value-of select="@policy"/></xsl:otherwise>
      </xsl:choose>
      <xsl:text> </xsl:text>
      <xsl:call-template name="counters"><xsl:with-param name="node" select="."/></xsl:call-template>
      <xsl:text>&#xA;</xsl:text>
    </xsl:for-each>
    <!-- Loop through each chain and output the rules -->
    <xsl:apply-templates select="node()"/>
    <xsl:text>COMMIT&#xA;# Completed&#xA;</xsl:text>
  </xsl:template>

  <xsl:template name="counters">
    <xsl:param name="node"/>
    <xsl:text>[</xsl:text>
    <xsl:if test="string-length($node/@packet-count)"><xsl:value-of select="$node/@packet-count"/></xsl:if>
    <xsl:if test="string-length($node/@packet-count)=0">0</xsl:if>
    <xsl:text>:</xsl:text>
    <xsl:if test="string-length($node/@byte-count)"><xsl:value-of select="$node/@byte-count"/></xsl:if>
    <xsl:if test="string-length($node/@byte-count)=0">0</xsl:if>
    <xsl:text>]</xsl:text>
  </xsl:template>

  <!-- the bit that automatically recurses for us, NOTE: we use * not node(), we don't want to copy every white space text -->
  <xsl:template match="@*|node()">
    <xsl:copy>
      <!-- with libxslt xsltproc we can't do @*|node() or the nodes may get processed before the attributes -->
      <xsl:apply-templates select="@*"/>
      <xsl:apply-templates select="node()"/>
    </xsl:copy>
  </xsl:template>

</xsl:transform>
